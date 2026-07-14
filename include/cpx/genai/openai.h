#ifndef CPX_GENAI_OPENAI_H
#define CPX_GENAI_OPENAI_H

#include <cpx/tag_info.h>
#include <string>
#include <vector>
#include <variant>

namespace cpx::genai::openai {
    CPX_EXPORT struct Content {
        std::string type, text; // TODO: other types?

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Content::type> = "type", //
            cpx::field<&Content::text> = "text,oneof=text|image|audio"
        );

        bool empty() const {
            return type.empty() && text.empty();
        }
    };

    CPX_EXPORT struct Message {
        std::string                                     role;
        std::variant<std::string, std::vector<Content>> content;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Message::role>    = "role   ,skipmissing,omitempty",
            cpx::field<&Message::content> = "content,skipmissing,omitempty"
        );

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

    CPX_EXPORT struct Choice {
        int         index = 0;
        Message     message, delta;
        std::string finish_reason;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Choice::index>         = "index",
            cpx::field<&Choice::message>       = "message       , oneof=message|delta",
            cpx::field<&Choice::delta>         = "delta         , oneof=message|delta",
            cpx::field<&Choice::finish_reason> = "finish_reason , skipmissing , omitempty"
        );
    };

    CPX_EXPORT struct Usage {
        int prompt_tokens     = 0;
        int completion_tokens = 0;
        int input_tokens      = 0;
        int output_tokens     = 0;
        int total_tokens      = 0;

        bool empty() const {
            return total_tokens == 0;
        }

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Usage::prompt_tokens>     = "prompt_tokens     , skipmissing , omitempty",
            cpx::field<&Usage::completion_tokens> = "completion_tokens , skipmissing , omitempty",
            cpx::field<&Usage::input_tokens>      = "input_tokens      , skipmissing , omitempty",
            cpx::field<&Usage::output_tokens>     = "output_tokens     , skipmissing , omitempty",
            cpx::field<&Usage::total_tokens>      = "total_tokens"
        );
    };

    CPX_EXPORT struct OutputItem {
        std::string          id;
        std::string          type;
        std::string          role;
        std::vector<Content> content;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&OutputItem::id>      = "id",
            cpx::field<&OutputItem::type>    = "type",
            cpx::field<&OutputItem::role>    = "role",
            cpx::field<&OutputItem::content> = "content"
        );
    };

    CPX_EXPORT struct Error {
        std::string message, type, param, code;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&Error::message> = "message",
            cpx::field<&Error::type>    = "type",
            cpx::field<&Error::param>   = "param    , skipmissing , omitempty",
            cpx::field<&Error::code>    = "code     , skipmissing , omitempty"
        );

        bool empty() const {
            return message.empty();
        }
    };

    CPX_EXPORT struct ChatCompletionsRequest {
        std::string          model;
        std::vector<Message> messages;
        bool                 stream = false;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&ChatCompletionsRequest::model>    = "model",
            cpx::field<&ChatCompletionsRequest::messages> = "messages",
            cpx::field<&ChatCompletionsRequest::stream>   = "stream,skipmissing,omitempty"
        );
    };

    CPX_EXPORT struct ChatCompletionsResponse {
        std::string         id, object, model;
        int64_t             created = 0;
        std::vector<Choice> choices;
        Usage               usage;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&ChatCompletionsResponse::id>      = "id",
            cpx::field<&ChatCompletionsResponse::model>   = "model",
            cpx::field<&ChatCompletionsResponse::choices> = "choices",
            cpx::field<&ChatCompletionsResponse::object>  = "object,skipmissing,omitempty",
            cpx::field<&ChatCompletionsResponse::created> = "created,skipmissing,omitempty",
            cpx::field<&ChatCompletionsResponse::usage>   = "usage,skipmissing,omitempty"
        );
    };

    CPX_EXPORT struct ChatCompletionsError {
        Error error;

        static constexpr auto __field_tags__ = std::make_tuple(cpx::field<&ChatCompletionsError::error> = "error");
    };

    CPX_EXPORT struct ResponsesRequest {
        std::string                                     model;
        std::variant<std::string, std::vector<Message>> input;
        bool                                            stream = false;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&ResponsesRequest::model>  = "model",
            cpx::field<&ResponsesRequest::input>  = "input",
            cpx::field<&ResponsesRequest::stream> = "stream,skipmissing,omitempty"
        );
    };

    CPX_EXPORT struct ResponsesResponse {
        std::string             id;
        std::string             object;
        int64_t                 created_at   = 0;
        int64_t                 completed_at = 0;
        std::string             model;
        std::vector<OutputItem> output;
        Usage                   usage;

        static constexpr auto __field_tags__ = std::make_tuple(
            cpx::field<&ResponsesResponse::id>           = "id",
            cpx::field<&ResponsesResponse::model>        = "model",
            cpx::field<&ResponsesResponse::output>       = "output",
            cpx::field<&ResponsesResponse::object>       = "object,skipmissing,omitempty",
            cpx::field<&ResponsesResponse::created_at>   = "created_at,skipmissing,omitempty",
            cpx::field<&ResponsesResponse::completed_at> = "completed_at,skipmissing,omitempty",
            cpx::field<&ResponsesResponse::usage>        = "usage,skipmissing,omitempty"
        );
    };

    CPX_EXPORT using ResponsesError = ChatCompletionsError;
} // namespace cpx::genai::openai

namespace cpx {
    CPX_EXPORT namespace openai = cpx::genai::openai;
}

#endif
