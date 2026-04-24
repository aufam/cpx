#ifndef CPX_INFERENCE_RUNTIME_H
#define CPX_INFERENCE_RUNTIME_H

#include <cpx/inference/config.h>
#include <cpx/inference/tensor.h>
#include <memory>

namespace cpx::inference {
    class Runtime {
    public:
        inference::Config cfg;

        Runtime()          = default;
        virtual ~Runtime() = default;

        virtual std::string version() const = 0;
        virtual void        apply_config()  = 0;

        virtual void load_model(const std::string &path) = 0;

        virtual std::vector<Tensor> get_inputs(const std::string &path) const  = 0;
        virtual std::vector<Tensor> get_outputs(const std::string &path) const = 0;

    protected:
        virtual std::shared_ptr<void>
        infer_backend(const std::string &path, const std::vector<Tensor> &inputs, std::vector<Tensor> &outputs) = 0;

    public:
        std::shared_ptr<void>
        infer(const std::string &path, const std::vector<Tensor> &expected_inputs, std::vector<Tensor> &expected_outputs) {
            auto input_tensors = get_inputs(path);
            if (input_tensors.size() > expected_inputs.size())
                throw std::runtime_error(
                    "Input tensors size mismatch. Expect " + std::to_string(expected_inputs.size()) + " at max, got " +
                    std::to_string(input_tensors.size())
                );

            auto compare_name = [](std::string_view a, std::string_view b) {
                auto sv = !b.empty() && b.back() == '?' ? b.substr(0, b.size() - 1) : b;
                for (size_t i; !sv.empty(); sv = sv.substr(i + 1)) {
                    i = sv.find('|');
                    if (a == sv.substr(0, i))
                        return true;
                    if (i == std::string::npos)
                        break;
                }
                return false;
            };

            auto compare = [&](const std::string &prefix, const Tensor &a, const Tensor &b) {
                if (!compare_name(a.name(), b.name()))
                    throw std::runtime_error(prefix + "name mismatch. Expect `" + b.name() + "`, got `" + a.name() + "`");

                if (a.element_type() != b.element_type())
                    throw std::runtime_error(
                        prefix + "element type mismatch. Expect `" + std::to_string((int)b.element_type()) + "`, got `" +
                        std::to_string((int)a.element_type()) + "`"
                    );

                if (a.shape().size() != b.shape().size())
                    throw std::runtime_error(
                        prefix + "shape size mismatch. Expect size=" + std::to_string(b.shape().size()) +
                        ", got size=" + std::to_string(a.shape().size())
                    );

                for (size_t i = 0; i < a.shape().size(); ++i) {
                    auto &dim_a = a.shape()[i];
                    auto &dim_b = b.shape()[i];
                    if (auto [pa, pb] = std::make_pair(
                            std::get_if<Tensor::StaticDimension>(&dim_a), std::get_if<Tensor::StaticDimension>(&dim_b)
                        );
                        pa && pb && *pa != *pb)
                        throw std::runtime_error(
                            prefix + "dimension mismatch at index=" + std::to_string(i) + ", expect " + std::to_string(*pb) +
                            " got " + std::to_string(*pa)
                        );
                    else if (auto [pa, pb] = std::make_pair(std::get_if<std::string>(&dim_a), std::get_if<std::string>(&dim_b));
                             pa && pb && *pa != *pb)
                        throw std::runtime_error(
                            prefix + "dimension mismatch at index=" + std::to_string(i) + ", expect " + *pb + " got " + *pa
                        );
                }
            };

            size_t i;
            for (i = 0; i < input_tensors.size(); ++i) {
                auto &tensor          = input_tensors[i];
                auto &expected_tensor = expected_inputs[i];
                compare("Input tensor ", tensor, expected_tensor);

                tensor.set_shape(expected_tensor.get_shape());
                tensor.set_raw_data(expected_tensor._data);
            }
            for (; i < expected_inputs.size(); ++i)
                if (auto &name = expected_inputs[i].name(); name.back() != '?')
                    throw std::runtime_error("Input tensors mismatch. Unknown non-optional tensor name `" + name + "`");

            auto output_tensors = get_outputs(path);
            auto defer          = infer_backend(path, input_tensors, output_tensors);
            if (output_tensors.size() > expected_outputs.size())
                throw std::runtime_error(
                    "Output tensors size mismatch. Expect " + std::to_string(expected_outputs.size()) + " at max, got " +
                    std::to_string(output_tensors.size())
                );

            for (i = 0; i < output_tensors.size(); ++i) {
                auto &tensor          = output_tensors[i];
                auto &expected_tensor = expected_outputs[i];
                compare("Output tensor ", tensor, expected_tensor);
            }
            for (; i < expected_outputs.size(); ++i)
                if (auto &name = expected_outputs[i].name(); name.back() != '?')
                    throw std::runtime_error("Output tensors mismatch. Unknown non-optional tensor name `" + name + "`");

            expected_outputs = std::move(output_tensors);
            return defer;
        }

        std::pair<std::vector<Tensor>, std::shared_ptr<void>>
        infer(const std::string &path, const std::vector<Tensor> &expected_inputs) {
            auto expected_outputs = get_outputs(path);
            auto defer            = infer(path, expected_inputs, expected_outputs);
            return {std::move(expected_outputs), std::move(defer)};
        }
    };
} // namespace cpx::inference

#endif
