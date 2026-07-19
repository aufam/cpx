#ifndef CPX_INFERENCE_OPENVINORUNTIME_H
#define CPX_INFERENCE_OPENVINORUNTIME_H

#include <openvino/openvino.hpp>
#include <cpx/inference/runtime.h>


namespace cpx::inference {
    CPX_EXPORT class OpenVINO : public Runtime {
    protected:
        ov::Core                                           core;
        std::unordered_map<std::string, ov::CompiledModel> models;

    public:
        OpenVINO()           = default;
        ~OpenVINO() override = default;

        std::string version() const override final {
            return std::string("OpenVINO v") + ov::get_openvino_version().buildNumber;
        }

        inline void apply_config() override final;

        void load_model(const std::string &path, const std::string &name = "") override final {
            models[name.empty() ? path : name] = core.compile_model(path, cfg.ov.device);
        }

        [[nodiscard]]
        std::vector<Tensor> get_inputs(const std::string &model) const override final {
            return get_ios(model, true);
        }

        [[nodiscard]]
        std::vector<Tensor> get_outputs(const std::string &model) const override final {
            return get_ios(model, false);
        }

        [[nodiscard]]
        inline std::shared_ptr<void>
        infer(const std::string &model, const std::vector<Tensor> &inputs, std::vector<Tensor> &outputs) override final;

    private:
        static Tensor::DynamicShape convert(const ov::PartialShape &shape) {
            Tensor::DynamicShape res(shape.size());
            for (size_t i = 0; i < shape.size(); ++i) {
                if (auto &dim = shape[std::ptrdiff_t(i)]; dim.is_dynamic())
                    res[i] = dim.to_string();
                else
                    res[i] = Tensor::StaticDimension(dim.get_length());
            }
            return res;
        }

        static ov::Tensor convert(const Tensor &t) {
            ov::element::Type type  = convert(t.element_type);
            ov::Shape         shape = t.get_shape();
            return {type, shape, t.get_raw_data()};
        }

        std::vector<Tensor> get_ios(const std::string &model_name, bool is_input) const {
            const auto &model = [&]() -> decltype(auto) {
                if (auto it = models.find(model_name); it != models.end())
                    return it->second;
                else
                    throw std::runtime_error("Cannot find '" + model_name + "', forgot to load it?");
            }();

            auto src = is_input ? model.inputs() : model.outputs();
            auto des = std::vector<Tensor>(src.size());

            for (size_t i = 0; i < src.size(); ++i) {
                des[i].name         = src[i].get_any_name();
                des[i].element_type = convert(src[i].get_element_type());
                des[i].shape        = convert(src[i].get_partial_shape());
            }

            return des;
        }

        static Tensor::ElementType convert(const ov::element::Type &type) {
            switch (ov::element::Type_t(type)) {
            case ov::element::Type_t::dynamic: return Tensor::ElementType::dynamic;
            case ov::element::Type_t::boolean: return Tensor::ElementType::boolean;
            case ov::element::Type_t::i4: return Tensor::ElementType::i4;
            case ov::element::Type_t::i8: return Tensor::ElementType::i8;
            case ov::element::Type_t::u1: return Tensor::ElementType::u1;
            case ov::element::Type_t::u2: return Tensor::ElementType::u2;
            case ov::element::Type_t::u3: return Tensor::ElementType::u3;
            case ov::element::Type_t::u4: return Tensor::ElementType::u4;
            case ov::element::Type_t::u6: return Tensor::ElementType::u6;
            case ov::element::Type_t::u8: return Tensor::ElementType::u8;
            case ov::element::Type_t::nf4: return Tensor::ElementType::nf4;
            case ov::element::Type_t::f8e4m3: return Tensor::ElementType::f8e4m3;
            case ov::element::Type_t::f8e5m2: return Tensor::ElementType::f8e5m2;
            case ov::element::Type_t::f8e8m0: return Tensor::ElementType::f8e8m0;
            case ov::element::Type_t::f4e2m1: return Tensor::ElementType::f4e2m1;
            case ov::element::Type_t::i16: return Tensor::ElementType::i16;
            case ov::element::Type_t::u16: return Tensor::ElementType::u16;
            case ov::element::Type_t::bf16: return Tensor::ElementType::bf16;
            case ov::element::Type_t::f16: return Tensor::ElementType::f16;
            case ov::element::Type_t::i32: return Tensor::ElementType::i32;
            case ov::element::Type_t::u32: return Tensor::ElementType::u32;
            case ov::element::Type_t::f32: return Tensor::ElementType::f32;
            case ov::element::Type_t::i64: return Tensor::ElementType::i64;
            case ov::element::Type_t::u64: return Tensor::ElementType::u64;
            case ov::element::Type_t::f64: return Tensor::ElementType::f64;
            case ov::element::Type_t::string: return Tensor::ElementType::string;
            }
            throw std::runtime_error("Unknown type: " + type.to_string());
        }

        static ov::element::Type convert(Tensor::ElementType type) {
            switch (type) {
            case Tensor::ElementType::dynamic: return ov::element::dynamic;
            case Tensor::ElementType::boolean: return ov::element::boolean;
            case Tensor::ElementType::i4: return ov::element::i4;
            case Tensor::ElementType::i8: return ov::element::i8;
            case Tensor::ElementType::u1: return ov::element::u1;
            case Tensor::ElementType::u2: return ov::element::u2;
            case Tensor::ElementType::u3: return ov::element::u3;
            case Tensor::ElementType::u4: return ov::element::u4;
            case Tensor::ElementType::u6: return ov::element::u6;
            case Tensor::ElementType::u8: return ov::element::u8;
            case Tensor::ElementType::nf4: return ov::element::nf4;
            case Tensor::ElementType::f8e4m3: return ov::element::f8e4m3;
            case Tensor::ElementType::f8e5m2: return ov::element::f8e5m2;
            case Tensor::ElementType::f8e8m0: return ov::element::f8e8m0;
            case Tensor::ElementType::f4e2m1: return ov::element::f4e2m1;
            case Tensor::ElementType::i16: return ov::element::i16;
            case Tensor::ElementType::u16: return ov::element::u16;
            case Tensor::ElementType::bf16: return ov::element::bf16;
            case Tensor::ElementType::f16: return ov::element::f16;
            case Tensor::ElementType::i32: return ov::element::i32;
            case Tensor::ElementType::u32: return ov::element::u32;
            case Tensor::ElementType::f32: return ov::element::f32;
            case Tensor::ElementType::i64: return ov::element::i64;
            case Tensor::ElementType::u64: return ov::element::u64;
            case Tensor::ElementType::f64: return ov::element::f64;
            case Tensor::ElementType::string: return ov::element::string;
            }
            throw std::runtime_error("Unknown type: " + std::to_string(int(type)));
        }
    };

    inline std::shared_ptr<void>
    OpenVINO::infer(const std::string &path, const std::vector<Tensor> &inputs, std::vector<Tensor> &outputs) {
        auto &model = models.at(path);
        auto  infer = model.create_infer_request();

        for (auto &t : inputs)
            infer.set_tensor(t.name, convert(t));

        infer.infer();

        std::vector<ov::Tensor *> ov_outputs;
        for (auto &t : outputs) {
            auto tensor = new ov::Tensor(infer.get_tensor(t.name));
            t.set_shape(tensor->get_shape());
            t.set_raw_data(tensor->data());
            ov_outputs.push_back(tensor);
        }

        return {
            new std::vector<ov::Tensor *>(std::move(ov_outputs)),
            [](void *ptr) {
                auto vec = static_cast<std::vector<ov::Tensor *> *>(ptr);
                for (auto t : *vec)
                    delete t;
                delete vec;
            },
        };
    }

    inline void OpenVINO::apply_config() {
        const auto &device = cfg.ov.device;

        // ---------- Threads ----------
        if (auto n = cfg.intra_threads) {
            core.set_property(device, ov::inference_num_threads(n));
        }

        if (auto n = cfg.inter_threads) {
            core.set_property(device, ov::num_streams(n));
        }

        // ---------- Device ----------
        if (auto &p = cfg.ov.device_id; !p.empty()) {
            core.set_property(device, ov::device::id(p));
        }

        // ---------- CPU pinning ----------
        if (auto &p = cfg.ov.enable_cpu_pinning; p.has_value()) {
            core.set_property(device, ov::hint::enable_cpu_pinning(*p));
        }

        // ---------- Auto-batching ----------
        if (auto &p = cfg.ov.allow_auto_batch; p.has_value()) {
            core.set_property(device, ov::hint::allow_auto_batching(*p));
        }

        // ---------- Optimization mode ----------
        switch (cfg.ov.optimization) {
        case Config::Ov::Optimization::LowLatency:
            core.set_property(device, ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY));
            break;

        case Config::Ov::Optimization::HighPerformance:
            core.set_property(device, ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT));
            break;

        case Config::Ov::Optimization::Balanced:
            core.set_property(device, ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT));
            break;

        default: break;
        }

        // ---------- Accuracy vs performance ----------
        switch (cfg.ov.prefered_execution_mode) {
        case Config::Ov::ExecutionMode::Accuracy:
            core.set_property(device, ov::hint::execution_mode(ov::hint::ExecutionMode::ACCURACY));
            break;

        case Config::Ov::ExecutionMode::Performance:
            core.set_property(device, ov::hint::execution_mode(ov::hint::ExecutionMode::PERFORMANCE));
            break;

        default: break;
        };
    }

} // namespace cpx::inference
#endif
