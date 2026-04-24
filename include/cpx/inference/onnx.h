#ifndef CPX_INFERENCE_ONNXRUNTIME_H
#define CPX_INFERENCE_ONNXRUNTIME_H

#include <onnxruntime_cxx_api.h>
#include <cpx/inference/runtime.h>
#include <unordered_map>

#ifdef _WIN32
#    include <codecvt>
#    include <locale>
#endif

namespace cpx::inference {

    class OnnxRuntime : public Runtime {
    protected:
        Ort::Env                                                       env;
        Ort::RunOptions                                                inference_opts;
        Ort::SessionOptions                                            model_opts;
        mutable Ort::AllocatorWithDefaultOptions                       allocator;
        std::unordered_map<std::string, std::unique_ptr<Ort::Session>> models;

    public:
        OnnxRuntime()
            : env(ORT_LOGGING_LEVEL_ERROR, "cpx:OnnxRuntime") {}

        ~OnnxRuntime() override = default;

        std::string version() const override {
            return std::string("OnnxRuntime v") + OrtGetApiBase()->GetVersionString();
        }

        void apply_config() override {
            const auto &c = cfg.ort();

            // -------- Logging --------
            if (c.verbose()) {
                env = Ort::Env{ORT_LOGGING_LEVEL_VERBOSE, "Cpx:OnnxRuntime"};
            }

            // -------- Session-level options --------
            model_opts = Ort::SessionOptions{};
            model_opts.SetGraphOptimizationLevel(static_cast<GraphOptimizationLevel>(c.graph_optimization_level()));

            // Disable CPU memory arena
            if (!c.enable_cpu_mem_arena()) {
                model_opts.DisableMemPattern();
                model_opts.DisableCpuMemArena();
            }

            // ------- Thread settings --------
            // (ORT naming differs from OV)
            if (cfg.intra_threads().has_value())
                model_opts.SetIntraOpNumThreads(*cfg.intra_threads());

            if (cfg.inter_threads().has_value())
                model_opts.SetInterOpNumThreads(*cfg.inter_threads());

            // -------- Execution providers --------
            if (c.use_cuda()) {
                OrtCUDAProviderOptions cuda_options{};
                cuda_options.device_id = (int)c.device_id();
                if (auto &p = cfg.gpu_mem_limit(); p.has_value())
                    cuda_options.gpu_mem_limit = Config::parse_mem_limit(*p);

                model_opts.AppendExecutionProvider_CUDA(cuda_options);
            }

            if (c.verbose()) {
                inference_opts.SetRunLogVerbosityLevel(1);
            }
        }

        void load_model(const std::string &path) override {
#ifdef _WIN32
            auto conv    = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{};
            auto wpath   = conv.from_bytes(path);
            models[path] = std::make_unique<Ort::Session>(env, wpath.c_str(), model_opts);
#else
            models[path] = std::make_unique<Ort::Session>(env, path.c_str(), model_opts);
#endif
        }

        std::vector<Tensor> get_inputs(const std::string &path) const override {
            return get_ios(path, true);
        }

        std::vector<Tensor> get_outputs(const std::string &path) const override {
            return get_ios(path, false);
        }

        std::shared_ptr<void>
        infer_backend(const std::string &path, const std::vector<Tensor> &inputs, std::vector<Tensor> &outputs) override {
            auto &model = *models.at(path);
            auto  mem   = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            std::vector<const char *> input_names;
            std::vector<const char *> output_names;

            std::vector<Ort::Value> input_tensors;
            for (auto &t : inputs) {
                input_names.push_back(t.name().c_str());
                auto shape = t.get_shape<int64_t>();
                input_tensors.push_back(Ort::Value::CreateTensor(
                    mem, t.get_raw_data(), t.get_size_in_bytes(), shape.data(), shape.size(), convert(t.element_type())
                ));
            }

            output_names.reserve(outputs.size());
            for (auto &t : outputs)
                output_names.push_back(t.name().c_str());

            std::vector<Ort::Value> onnx_outputs = model.Run(
                inference_opts, input_names.data(), input_tensors.data(), inputs.size(), output_names.data(), output_names.size()
            );

            for (size_t i = 0; i < onnx_outputs.size(); i++) {
                auto &src = onnx_outputs[i];
                auto &des = outputs[i];

                auto info = src.GetTensorTypeAndShapeInfo();
                des.set_shape(info.GetShape());
                des.set_raw_data(src.GetTensorMutableData<void>());
            }

            auto ptr = new std::vector<Ort::Value>(std::move(onnx_outputs));
            return {ptr, [](void *ptr) { delete static_cast<std::vector<Ort::Value> *>(ptr); }};
        }

    protected:
        static std::vector<std::string> get_node_names(
            const Ort::Session *session, size_t names_size, bool is_input, Ort::AllocatorWithDefaultOptions &allocator
        ) {
            std::vector<std::string> res;
#if ORT_API_VERSION < 13
            for (size_t i = 0; i < names_size; ++i) {
                char *input_name = is_input ? session->GetInputName(i, allocator) : session->GetOutputName(i, allocator);
                res.emplace_back(input_name);
                allocator.Free(input_name);
            }
#else
            for (size_t i = 0; i < names_size; ++i) {
                auto ptr =
                    is_input ? session->GetInputNameAllocated(i, allocator) : session->GetOutputNameAllocated(i, allocator);
                res.emplace_back(ptr.get());
            }
#endif
            return res;
        }

        std::vector<Tensor> get_ios(const std::string &path, bool is_input) const {
            const auto &model = [&]() -> decltype(auto) {
                if (auto it = models.find(path); it != models.end())
                    return *it->second;
                else
                    throw std::runtime_error("'" + path + "' was not loaded");
            }();

            auto count = is_input ? model.GetInputCount() : model.GetOutputCount();
            auto names = get_node_names(&model, count, is_input, allocator);
            auto res   = std::vector<Tensor>(count);

            for (size_t i = 0; i < count; i++) {
                auto info  = is_input ? model.GetInputTypeInfo(i) : model.GetOutputTypeInfo(i);
                auto type  = info.GetTensorTypeAndShapeInfo().GetElementType();
                auto shape = info.GetTensorTypeAndShapeInfo().GetShape();

                auto &t          = res[i];
                t.name()         = names[i];
                t.element_type() = convert(type);
                t.shape().resize(shape.size());

                for (size_t j = 0; j < shape.size(); ++j)
                    if (auto dim = shape[j]; dim < 0)
                        t.shape()[j] = "?";
                    else
                        t.shape()[j] = Tensor::StaticDimension(dim);
            }

            return res;
        }

        static ONNXTensorElementDataType convert(Tensor::ElementType t) {
            switch (t) {
            case Tensor::ElementType::boolean:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
            case Tensor::ElementType::f16:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
            case Tensor::ElementType::bf16:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16;
            case Tensor::ElementType::f32:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
            case Tensor::ElementType::f64:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE;
            case Tensor::ElementType::i8:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
            case Tensor::ElementType::i16:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16;
            case Tensor::ElementType::i32:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
            case Tensor::ElementType::i64:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
            case Tensor::ElementType::u8:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8;
            case Tensor::ElementType::u16:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16;
            case Tensor::ElementType::u32:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32;
            case Tensor::ElementType::u64:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64;
            case Tensor::ElementType::string:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;

            //
            // Unsupported by ONNX Runtime
            //
            case Tensor::ElementType::dynamic:
            case Tensor::ElementType::i4:
            case Tensor::ElementType::u1:
            case Tensor::ElementType::u2:
            case Tensor::ElementType::u3:
            case Tensor::ElementType::u4:
            case Tensor::ElementType::u6:
            case Tensor::ElementType::nf4:
            case Tensor::ElementType::f8e4m3:
            case Tensor::ElementType::f8e5m2:
            case Tensor::ElementType::f4e2m1:
            case Tensor::ElementType::f8e8m0:
            default:
                return ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
            }
        }

        static Tensor::ElementType convert(ONNXTensorElementDataType ort_type) {
            switch (ort_type) {
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
                return Tensor::ElementType::boolean;

            case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
                return Tensor::ElementType::f16;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16:
                return Tensor::ElementType::bf16;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
                return Tensor::ElementType::f32;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
                return Tensor::ElementType::f64;

            case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
                return Tensor::ElementType::i8;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
                return Tensor::ElementType::i16;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
                return Tensor::ElementType::i32;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
                return Tensor::ElementType::i64;

            case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
                return Tensor::ElementType::u8;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
                return Tensor::ElementType::u16;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
                return Tensor::ElementType::u32;
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64:
                return Tensor::ElementType::u64;

            case ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING:
                return Tensor::ElementType::string;

            //
            // NOT SUPPORTED BY ONNX RUNTIME
            // fp8 formats, nf4, i4, u1/u2/u3/u4/u6, etc
            //
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED:
            default:
                return Tensor::ElementType::dynamic;
            }
        }
    };
} // namespace cpx::inference
#endif
