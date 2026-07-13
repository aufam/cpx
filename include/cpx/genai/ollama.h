#ifndef CPX_GENAI_OLLAMA_H
#define CPX_GENAI_OLLAMA_H

#include <cpx/tag_info.h>
#include <string>

namespace cpx::genai::ollama {
    CPX_EXPORT struct GenerateContentRequest {
        std::string prompt, model;
        bool        stream = false;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&GenerateContentRequest::prompt> = "prompt",
            cpx::field<&GenerateContentRequest::model>  = "model",
            cpx::field<&GenerateContentRequest::stream> = "stream,skipmissing,omitempty"
        );
    };

    CPX_EXPORT struct GenerateContentResponse {
        std::string response;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&GenerateContentResponse::response> = "response" //
        );
    };

    CPX_EXPORT struct Error {
        std::string error;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Error::error> = "error" //
        );
    };
} // namespace cpx::genai::ollama

namespace cpx {
    CPX_EXPORT namespace ollama = cpx::genai::ollama;
}

#endif
