#ifndef CPX_GENAI_OPENAI_CHAT_COMPLETIONS_H
#define CPX_GENAI_OPENAI_CHAT_COMPLETIONS_H

#include <cpx/reflect.h>
#include <string>
#include <vector>
#include <variant>

namespace cpx::genai::openai {
    struct Content {
        std::string type, text; // TODO: other types?

        bool empty() const {
            return type.empty() && text.empty();
        }
    };

    struct Message {
        std::string                                     role;
        std::variant<std::string, std::vector<Content>> content;

        bool empty() const {
            return role.empty() && std::visit([](const auto &c) { return c.empty(); }, content);
        }

        const std::string &get_text() const {
            if (std::holds_alternative<std::string>(content)) {
                return std::get<std::string>(content);
            } else {
                return std::get<std::vector<Content>>(content).at(0).text;
            }
        }
    };

    struct Choice {
        int         index = 0;
        Message     message, delta;
        std::string finish_reason;
    };

    struct Usage {
        int prompt_tokens     = 0;
        int completion_tokens = 0;
        int total_tokens      = 0;

        bool empty() const {
            return total_tokens == 0;
        }
    };

    struct Error {
        std::string message, type, param, code;

        bool empty() const {
            return message.empty();
        }
    };

    struct OutputItem {
        std::string          id;
        std::string          type;
        std::string          role;
        std::vector<Content> content;
    };

    struct ChatCompletionsRequest {
        std::string          model;
        std::vector<Message> messages;
        bool                 stream = false;
    };

    struct ChatCompletionsResponse {
        std::string         id, object, model;
        int64_t             created = 0;
        std::vector<Choice> choices;
        Usage               usage;
    };

    struct ResponsesRequest {
        std::string                                     model;
        std::variant<std::string, std::vector<Message>> input;
        bool                                            stream = false;
    };

    struct ResponsesResponse {
        std::string             id;
        std::string             object;
        int64_t                 created_at   = 0;
        int64_t                 completed_at = 0;
        std::string             model;
        std::vector<OutputItem> output;
        Usage                   usage;
    };
} // namespace cpx::genai::openai

#define NS cpx::genai::openai

template <>
struct cpx::Reflect<NS::Content> : cpx::Fields<cpx::Reflect<NS::Content>, &NS::Content::type, &NS::Content::text> {
    static constexpr TagInfo type = "type";
    static constexpr TagInfo text = "text,oneof=text|image|audio";

    static constexpr tags_type tags() {
        return std::tie(type, text);
    }
};

template <>
struct cpx::Reflect<NS::Message> : cpx::Fields<cpx::Reflect<NS::Message>, &NS::Message::role, &NS::Message::content> {
    static constexpr TagInfo role    = "role,skipmissing,omitempty";
    static constexpr TagInfo content = "content,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(role, content);
    }
};

template <>
struct cpx::Reflect<NS::Choice> : cpx::Fields<
                                      cpx::Reflect<NS::Choice>,
                                      &NS::Choice::index,
                                      &NS::Choice::message,
                                      &NS::Choice::delta,
                                      &NS::Choice::finish_reason> {
    static constexpr TagInfo index         = "index";
    static constexpr TagInfo message       = "message,oneof=message|delta";
    static constexpr TagInfo delta         = "delta,oneof=message|delta";
    static constexpr TagInfo finish_reason = "finish_reason,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(index, message, delta, finish_reason);
    }
};

template <>
struct cpx::Reflect<NS::Usage>
    : cpx::Fields<cpx::Reflect<NS::Usage>, &NS::Usage::prompt_tokens, &NS::Usage::completion_tokens, &NS::Usage::total_tokens> {
    static constexpr TagInfo prompt_tokens     = "prompt_tokens,skipmissing,omitempty";
    static constexpr TagInfo completion_tokens = "completion_tokens,skipmissing,omitempty";
    static constexpr TagInfo total_tokens      = "total_tokens";

    static constexpr tags_type tags() {
        return std::tie(prompt_tokens, completion_tokens, total_tokens);
    }
};

template <>
struct cpx::Reflect<NS::OutputItem> : cpx::Fields<
                                          cpx::Reflect<NS::OutputItem>,
                                          &NS::OutputItem::id,
                                          &NS::OutputItem::type,
                                          &NS::OutputItem::role,
                                          &NS::OutputItem::content> {
    static constexpr TagInfo id      = "id,skipmissing,omitempty";
    static constexpr TagInfo type    = "type";
    static constexpr TagInfo role    = "role,skipmissing,omitempty";
    static constexpr TagInfo content = "content";

    static constexpr tags_type tags() {
        return std::tie(id, type, role, content);
    }
};

template <>
struct cpx::Reflect<NS::Error>
    : cpx::Fields<cpx::Reflect<NS::Error>, &NS::Error::message, &NS::Error::type, &NS::Error::param, &NS::Error::code> {
    static constexpr TagInfo message = "message";
    static constexpr TagInfo type    = "type,omitempty";
    static constexpr TagInfo param   = "param,skipmissing,omitempty";
    static constexpr TagInfo code    = "code,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(message, type, param, code);
    }
};

template <>
struct cpx::Reflect<NS::ChatCompletionsRequest> : cpx::Fields<
                                                      cpx::Reflect<NS::ChatCompletionsRequest>,
                                                      &NS::ChatCompletionsRequest::model,
                                                      &NS::ChatCompletionsRequest::messages,
                                                      &NS::ChatCompletionsRequest::stream> {
    static constexpr TagInfo model    = "model";
    static constexpr TagInfo messages = "messages";
    static constexpr TagInfo stream   = "stream,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(model, messages, stream);
    }
};

template <>
struct cpx::Reflect<NS::ResponsesRequest> : cpx::Fields<
                                                cpx::Reflect<NS::ResponsesRequest>,
                                                &NS::ResponsesRequest::model,
                                                &NS::ResponsesRequest::input,
                                                &NS::ResponsesRequest::stream> {
    static constexpr TagInfo model  = "model";
    static constexpr TagInfo input  = "input";
    static constexpr TagInfo stream = "stream,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(model, input, stream);
    }
};

template <>
struct cpx::Reflect<NS::ChatCompletionsResponse> : cpx::Fields<
                                                       cpx::Reflect<NS::ChatCompletionsResponse>,
                                                       &NS::ChatCompletionsResponse::id,
                                                       &NS::ChatCompletionsResponse::object,
                                                       &NS::ChatCompletionsResponse::created,
                                                       &NS::ChatCompletionsResponse::model,
                                                       &NS::ChatCompletionsResponse::choices,
                                                       &NS::ChatCompletionsResponse::usage> {
    static constexpr TagInfo id      = "id,skipmissing,omitempty";
    static constexpr TagInfo object  = "object,skipmissing,omitempty";
    static constexpr TagInfo created = "created,skipmissing,omitempty";
    static constexpr TagInfo model   = "model,skipmissing,omitempty";
    static constexpr TagInfo choices = "choices";
    static constexpr TagInfo usage   = "usage,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(id, object, created, model, choices, usage);
    }
};

template <>
struct cpx::Reflect<NS::ResponsesResponse> : cpx::Fields<
                                                 cpx::Reflect<NS::ResponsesResponse>,
                                                 &NS::ResponsesResponse::id,
                                                 &NS::ResponsesResponse::object,
                                                 &NS::ResponsesResponse::created_at,
                                                 &NS::ResponsesResponse::completed_at,
                                                 &NS::ResponsesResponse::model,
                                                 &NS::ResponsesResponse::output,
                                                 &NS::ResponsesResponse::usage> {
    static constexpr TagInfo id           = "id,skipmissing,omitempty";
    static constexpr TagInfo object       = "object,skipmissing,omitempty";
    static constexpr TagInfo created_at   = "created_at,skipmissing,omitempty";
    static constexpr TagInfo completed_at = "completed_at,skipmissing,omitempty";
    static constexpr TagInfo model        = "model,skipmissing,omitempty";
    static constexpr TagInfo output       = "output";
    static constexpr TagInfo usage        = "usage,skipmissing,omitempty";

    static constexpr tags_type tags() {
        return std::tie(id, object, created_at, completed_at, model, output, usage);
    }
};
#undef NS
#endif
