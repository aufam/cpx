#ifndef CPX_GENAI_OPENAI_CHAT_COMPLETIONS_H
#define CPX_GENAI_OPENAI_CHAT_COMPLETIONS_H

#include <cpx/tag.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <ctime>

namespace cpx::genai::openai {
    struct ChatCompletionsRequest {
        struct Message {
            Tag<std::string> role    = "json:`role`";
            Tag<std::string> content = "json:`content`";
        };
        Tag<std::string>          model    = "json:`model`";
        Tag<std::vector<Message>> messages = "json:`messages`";
        Tag<bool>                 stream   = {"json:`stream`", false};

        static ChatCompletionsRequest
        create(const std::string &model, std::string prompt, std::string system_content = "You are a helpful assistant.") {
            ChatCompletionsRequest req;
            req.model() = model;
            req.messages().resize(2);
            req.messages()[0].role()    = "system";
            req.messages()[0].content() = std::move(system_content);
            req.messages()[1].role()    = "user";
            req.messages()[1].content() = std::move(prompt);
            return req;
        }
    };

    struct ChatCompletionsResponse {
        struct Choice {
            struct Message {
                Tag<std::string> content = "json:`content`";
            };
            struct Delta {
                Tag<std::string> content = "json:`content,skipmissing`";
            };
            Tag<Message> message = "json:`message,skipmissing`";
            Tag<Delta>   delta   = "json:`delta,skipmissing`";
        };

        struct Usage {
            struct PromptTokensDetail {
                Tag<int> cached_tokens = "json:cached_tokens,skipmissing";
                Tag<int> audio_tokens  = "json:audio_tokens,skipmissing";
            };

            struct CompletionTokensDetail {
                Tag<int> reasoning_tokens           = "json:reasoning_tokens,skipmissing";
                Tag<int> audio_tokens               = "json:audio_tokens,skipmissing";
                Tag<int> accepted_prediction_tokens = "json:accepted_prediction_tokens,skipmissing";
                Tag<int> rejected_prediction_tokens = "json:rejected_prediction_tokens,skipmissing";
            };

            Tag<int>                    prompt_tokens            = "json:prompt_tokens";
            Tag<int>                    completion_tokens        = "json:completion_tokens";
            Tag<int>                    total_tokens             = "json:total_tokens";
            Tag<PromptTokensDetail>     prompt_tokens_detail     = "json:prompt_tokens_detail";
            Tag<CompletionTokensDetail> completion_tokens_detail = "json:completion_tokens_detail";
        };

        struct Error {
            Tag<std::string> code    = "json:`code`";
            Tag<std::string> type    = "json:`type`";
            Tag<std::string> message = "json:`message`";
        };

        Tag<std::string>          id      = "json:`id,skipmissing,omitempty`";
        Tag<std::string>          object  = "json:`object,skipmissing,omitempty`";
        Tag<std::time_t>          created = "json:`created,skipmissing,omitempty`";
        Tag<std::string>          model   = "json:`model,skipmissing,omitempty`";
        Tag<std::vector<Choice>>  choices = "json:`choices,skipmissing,omitempty`";
        Tag<std::optional<Error>> error   = "json:`error,omitempty`";

        const std::string &get_text() const & {
            if (error().has_value())
                throw std::runtime_error("openai chat completions error: " + error()->message());
            return choices().at(0).message().content();
        }

        std::string get_text() && {
            if (error().has_value())
                throw std::runtime_error("openai chat completions error: " + error()->message());
            return std::move(choices().at(0).message().content());
        }
    };
} // namespace cpx::genai::openai

#endif
