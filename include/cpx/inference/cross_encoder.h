#ifndef CPX_INFERENCE_CROSS_ENCODER_H
#define CPX_INFERENCE_CROSS_ENCODER_H

#include <cpx/inference/tensor.h>

namespace cpx::inference::cross_encoder {
    inline std::pair<std::vector<Tensor>, std::vector<Tensor>> build_ios(
        const std::vector<int64_t> &input_ids,
        const std::vector<int64_t> &attention_mask,
        const std::vector<int64_t> &token_type_ids,
        size_t                      batch,
        size_t                      seq_len,
        size_t                      output_classes_len
    ) {
        std::vector<inference::Tensor> inputs(3);
        std::vector<inference::Tensor> outputs(1);

        inputs[0].name()         = "input_ids";
        inputs[0].element_type() = inference::Tensor::ElementType::i64;
        inputs[0].set_shape({batch, seq_len});
        inputs[0].set_data(input_ids);

        inputs[1].name()         = "attention_mask";
        inputs[1].element_type() = inference::Tensor::ElementType::i64;
        inputs[1].set_shape({batch, seq_len});
        inputs[1].set_data(attention_mask);

        inputs[2].name()         = "token_type_ids?";
        inputs[2].element_type() = inference::Tensor::ElementType::i64;
        inputs[2].set_shape({batch, seq_len});
        inputs[2].set_data(token_type_ids);

        outputs[0].name()         = "logits|scores|output_0";
        outputs[0].element_type() = inference::Tensor::ElementType::f32;
        outputs[0].set_shape({batch, output_classes_len});

        return {std::move(inputs), std::move(outputs)};
    }
} // namespace cpx::inference::cross_encoder

#endif
