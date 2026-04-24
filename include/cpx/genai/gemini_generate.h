#ifndef CPX_GENAI_GEMINI_GENERATE_H
#define CPX_GENAI_GEMINI_GENERATE_H

#include <cpx/tag.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <optional>

namespace cpx::genai::gemini {
    struct Content {
        struct Part {
            Tag<std::string> text = "json:`text`";
        };
        Tag<std::vector<Part>>          parts = "json:`parts`";
        Tag<std::optional<std::string>> role  = "json:`role`";
    };

    struct GenerateContentRequest {
        Tag<std::vector<Content>> contents = "json:`contents`";

        static GenerateContentRequest create(std::string prompt, const std::optional<std::string> &role = std::nullopt) {
            GenerateContentRequest req;
            req.contents().resize(1);
            req.contents()[0].role() = role;
            req.contents()[0].parts().resize(1);
            req.contents()[0].parts()[0].text() = std::move(prompt);
            return req;
        }

        template <typename Message>
        static GenerateContentRequest create(std::vector<Message> &&messages) {
            size_t size = messages.size();

            GenerateContentRequest req;
            req.contents().resize(size);

            for (size_t i = 0; i < size; ++i) {
                auto &src  = messages[i];
                auto &dest = req.contents()[i];

                dest.role() = src.role() == "user" ? "user" : "model"; // only "user" and "model" are allowed in Gemini API
                dest.parts().resize(1);
                dest.parts()[0].text() = std::move(src.content());
            }
            return req;
        }
    };

    struct GenerateContentResponse {
        struct Candidate {
            Tag<Content>     content       = "json:`content`";
            Tag<std::string> finish_reason = "json:`finishReason,skipmissing`";
            Tag<size_t>      index         = "json:`index,skipmissing`";
        };

        struct UsageMetadata {
            struct PromptTokensDetail {
                Tag<std::string> modality    = "json:`modality`";
                Tag<int>         token_count = "json:`tokenCount`";
            };
            Tag<int>                             prompt_token_count     = "json:`promtTokenCount`";
            Tag<int>                             candidates_token_count = "json:`candidatesTokenCount`";
            Tag<int>                             total_token_count      = "json:`totalTokenCount`";
            Tag<int>                             thoughts_token_count   = "json:`thoughtsTokenCount`";
            Tag<std::vector<PromptTokensDetail>> prompt_tokens_details  = "json:`promptTokensDetails`";
        };

        struct Error {
            struct Detail {
                struct Metadata {
                    Tag<std::string> service = "json:`service`";
                };

                Tag<std::string> type     = "json:`@type`";
                Tag<std::string> reason   = "json:`reason,skipmissing`";
                Tag<std::string> domain   = "json:`domain,skipmissing`";
                Tag<Metadata>    metadata = "json:`metadata,skipmissing`";
                Tag<std::string> locale   = "json:`locale,skipmissing`";
                Tag<std::string> message  = "json:`message,skipmissing`";
            };

            Tag<int>                 code    = "json:`code,skipmissing`";
            Tag<std::string>         message = "json:`message`";
            Tag<std::string>         status  = "json:`status,skipmissing`";
            Tag<std::vector<Detail>> details = "json:`details,skipmissing`";
        };

        Tag<std::vector<Candidate>> candidates      = "json:`candidates,skipmissing,omitempty`";
        Tag<UsageMetadata>          usage_meta_data = "json:`usageMetaData,skipmissing,omitempty`";
        Tag<std::string>            model_version   = "json:`modelVersion,skipmissing,omitempty`";
        Tag<std::string>            response_id     = "json:`responseId,skipmissing,omitempty`";
        Tag<std::optional<Error>>   error           = "json:`error,omitempty`";

        const std::string &get_text() const & {
            if (error().has_value())
                throw std::runtime_error("genai generate content error: " + error()->message());
            return candidates().at(0).content().parts().at(0).text();
        }

        std::string get_text() && {
            if (error().has_value())
                throw std::runtime_error("genai generate content error: " + error()->message());
            return std::move(candidates().at(0).content().parts().at(0).text());
        }
    };
} // namespace cpx::genai::gemini

#endif
