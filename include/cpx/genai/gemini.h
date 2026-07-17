#ifndef CPX_GENAI_GEMINI_H
#define CPX_GENAI_GEMINI_H

#include <cpx/tag_info.h>
#include <string>
#include <utility>
#include <vector>
#include <tuple>

namespace cpx::genai::gemini {
    CPX_EXPORT struct Content {
        struct Part {
            std::string text;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Part::text> = "text",
            };
        };

        std::vector<Part> parts;
        std::string       role;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Content::parts> = "parts",
            cpx::field<&Content::role>  = "role,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Content>);
    static_assert(std::is_default_constructible_v<Content::Part>);

    struct Candidate {
        size_t      index = 0;
        Content     content;
        std::string finish_reason;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Candidate::index>         = "index",
            cpx::field<&Candidate::content>       = "content",
            cpx::field<&Candidate::finish_reason> = "finishReason,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Candidate>);

    struct UsageMetaData {
        struct PromptTokensDetail {
            std::string modality;
            int         token_count = 0;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&PromptTokensDetail::modality>    = "modality",
                cpx::field<&PromptTokensDetail::token_count> = "tokenCount"
            };
        };

        int                             prompt_token_count     = 0;
        int                             candidates_token_count = 0;
        int                             total_token_count      = 0;
        int                             thoughts_token_count   = 0;
        std::vector<PromptTokensDetail> prompt_tokens_details;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&UsageMetaData::prompt_token_count>     = "promtTokenCount",
            cpx::field<&UsageMetaData::candidates_token_count> = "candidatesTokenCount",
            cpx::field<&UsageMetaData::total_token_count>      = "totalTokenCount",
            cpx::field<&UsageMetaData::thoughts_token_count>   = "thoughtsTokenCount",
            cpx::field<&UsageMetaData::prompt_tokens_details>  = "promptTokensDetails",
        };
    };
    static_assert(std::is_default_constructible_v<UsageMetaData>);
    static_assert(std::is_default_constructible_v<UsageMetaData::PromptTokensDetail>);

    struct Error {
        struct Detail {
            struct Metadata {
                std::string service;

                static constexpr std::tuple __field_tags__ = {
                    cpx::field<&Metadata::service> = "service",
                };
            };

            std::string type;
            std::string reason;
            std::string domain;
            Metadata    metadata;
            std::string locale;
            std::string message;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Detail::type>     = "@type",
                cpx::field<&Detail::reason>   = "reason",
                cpx::field<&Detail::domain>   = "domain",
                cpx::field<&Detail::metadata> = "metadata",
                cpx::field<&Detail::locale>   = "locale",
                cpx::field<&Detail::message>  = "message",
            };
        };

        int                 code = 0;
        std::string         message;
        std::string         status;
        std::vector<Detail> details;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Error::code>    = "code",
            cpx::field<&Error::message> = "message",
            cpx::field<&Error::status>  = "status",
            cpx::field<&Error::details> = "details,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Error>);
    static_assert(std::is_default_constructible_v<Error::Detail>);
} // namespace cpx::genai::gemini

namespace cpx::genai::gemini::generate_content {
    CPX_EXPORT struct Request {
        std::vector<Content> contents;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::contents> = "contents",
        };

        static Request create(std::string prompt, const std::string &role = "") {
            Request req;
            req.contents.resize(1);
            req.contents[0].role = role;
            req.contents[0].parts.resize(1);
            req.contents[0].parts[0].text = std::move(prompt);
            return req;
        }

        template <typename Message>
        static Request create(std::vector<Message> &&messages) {
            size_t size = messages.size();

            Request req;
            req.contents.resize(size);

            for (size_t i = 0; i < size; ++i) {
                auto &src  = messages[i];
                auto &dest = req.contents[i];

                dest.role = src.role == "user" ? "user" : "model"; // only "user" and "model" are allowed in Gemini API
                dest.parts.resize(1);
                dest.parts[0].text = std::move(src.get_text());
            }
            return req;
        }
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct Response {
        std::vector<Candidate> candidates;
        UsageMetaData          usage_meta_data;
        std::string            model_version;
        std::string            response_id;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Response::candidates>      = "candidates",
            cpx::field<&Response::model_version>   = "modelVersion",
            cpx::field<&Response::response_id>     = "responseId",
            cpx::field<&Response::usage_meta_data> = "usageMetaData",
        };

        const std::string &get_text() const & {
            return candidates.at(0).content.parts.at(0).text;
        }

        std::string get_text() && {
            return std::move(candidates.at(0).content.parts.at(0).text);
        }
    };
    static_assert(std::is_default_constructible_v<Response>);

    CPX_EXPORT struct ErrorResponse {
        Error error;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ErrorResponse::error> = "error",
        };
    };
    static_assert(std::is_default_constructible_v<Response>);
} // namespace cpx::genai::gemini::generate_content


namespace cpx {
    CPX_EXPORT namespace gemini = cpx::genai::gemini;
}

#endif
