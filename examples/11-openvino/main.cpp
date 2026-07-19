#include <cpx/inference/openvino.h>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace fs = std::filesystem;

struct Cli {
    std::string image;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Cli::image> = "image,positional",
    };
};

int main(int argc, char **argv) {
    const std::string model_name = "resnet18-v2-7";
    const std::string model      = model_name + ".onnx";

    if (!fs::exists(model)) {
        fmt::println(
            stderr,
            "missing {0}, please download it first: "
            "`wget https://huggingface.co/onnxmodelzoo/{1}/resolve/main/{0}?download=true -O {0}`",
            model,
            model_name
        );
        return 1;
    }

    Cli cli;
    cpx::cli11::parse("openvino example", argc, argv, cli);

    cv::Mat image = cv::imread(cli.image);
    cv::resize(image, image, {224, 224});

    cv::Mat blob = cv::dnn::blobFromImage(
        image,
        1.0 / 255.0, // scale to [0,1]
        cv::Size(224, 224),
        cv::Scalar(0, 0, 0), // no mean subtraction
        true,                // BGR -> RGB
        false,
        CV_32F
    );

    constexpr float mean[] = {0.485f, 0.456f, 0.406f};
    constexpr float std[]  = {0.229f, 0.224f, 0.225f};

    const int H = blob.size[2];
    const int W = blob.size[3];

    float *data = reinterpret_cast<float *>(blob.data);

    for (int c = 0; c < 3; ++c) {
        float *plane = data + c * H * W;

        for (int i = 0; i < H * W; ++i)
            plane[i] = (plane[i] - mean[c]) / std[c];
    }

    cpx::inference::OpenVINO ov;
    ov.load_model(model);

    auto inputs  = ov.get_inputs(model);
    auto outputs = ov.get_outputs(model);

    fmt::println(stderr, "inputs: {}", inputs);
    fmt::println(stderr, "outputs: {}", outputs);

    inputs[0].set_shape({1, 3, 224, 224});
    inputs[0].set_raw_data(data);

    auto _ = ov.infer(model, inputs, outputs);

    const float *scores = outputs[0].get_data<float>();
    const float *best   = std::max_element(scores, scores + outputs[0].get_size());

    fmt::println("Predicted class index = {}", std::distance(scores, best));
    fmt::println("Score = {}", *best);
}
