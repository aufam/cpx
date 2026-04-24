#ifndef CPX_GENAI_OLLAMA_GENERATE_H
#define CPX_GENAI_OLLAMA_GENERATE_H

#include <cpx/tag.h>
#include <stdexcept>
#include <string>

namespace cpx::genai::ollama {
    struct GenerateContentRequest {
        Tag<std::string> prompt = "json:`prompt`";
        Tag<std::string> model  = "json:`model`";
        Tag<bool>        stream = {"json:`stream`", false};
    };

    struct GenerateContentResponse {
        Tag<std::string> response = "json:`response,skipmissing,omitempty`";
        Tag<std::string> error    = "json:`error,skipmissing,omitempty`";

        const std::string &get_text() const {
            if (!error().empty())
                throw std::runtime_error("ollama generate content error: " + error());
            return response();
        }
    };

    template <typename Message>
    struct ChatCompletionsResponse {
        Tag<Message>     message = "json:`message,skipmissing`";
        Tag<std::string> error   = "json:`error,skipmissing,omitempty`";

        const std::string &get_text() const {
            if (!error().empty())
                throw std::runtime_error("ollama chat completions error: " + error());
            return message().content();
        }
    };
} // namespace cpx::genai::ollama

#endif
