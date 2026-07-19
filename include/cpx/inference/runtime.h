#ifndef CPX_INFERENCE_RUNTIME_H
#define CPX_INFERENCE_RUNTIME_H

#include <cpx/inference/config.h>
#include <cpx/inference/tensor.h>
#include <memory>

namespace cpx::inference {
    CPX_EXPORT class Runtime {
    public:
        inference::Config cfg;

        Runtime()          = default;
        virtual ~Runtime() = default;

        virtual auto version() const -> std::string = 0;
        virtual void apply_config()                 = 0;

        virtual void load_model(const std::string &path, const std::string &name = "") = 0;

        [[nodiscard]]
        virtual auto get_inputs(const std::string &path) const -> std::vector<Tensor> = 0;

        [[nodiscard]]
        virtual auto get_outputs(const std::string &path) const -> std::vector<Tensor> = 0;

        [[nodiscard]]
        virtual std::shared_ptr<void>
        infer(const std::string &model, const std::vector<Tensor> &inputs, std::vector<Tensor> &outputs) = 0;
    };
} // namespace cpx::inference

#endif
