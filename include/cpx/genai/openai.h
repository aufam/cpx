#ifndef CPX_GENAI_OPENAI_H
#define CPX_GENAI_OPENAI_H

#include <cpx/genai/openai_common.h>

namespace cpx::genai::openai {
    CPX_EXPORT struct Choice {
        int         index = 0;
        Message     message, delta;
        std::string finish_reason;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Choice::index>         = "index",
            cpx::field<&Choice::message>       = "message       , oneof=message|delta",
            cpx::field<&Choice::delta>         = "delta         , oneof=message|delta",
            cpx::field<&Choice::finish_reason> = "finish_reason , skipmissing , omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Choice>);

    CPX_EXPORT struct OutputItem {
        std::string          id;
        std::string          type;
        std::string          role;
        std::vector<Content> content;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&OutputItem::id>      = "id",
            cpx::field<&OutputItem::type>    = "type",
            cpx::field<&OutputItem::role>    = "role",
            cpx::field<&OutputItem::content> = "content"
        };
    };
    static_assert(std::is_default_constructible_v<OutputItem>);
} // namespace cpx::genai::openai

namespace cpx::genai::openai::chat_completions {
    CPX_EXPORT struct Request {
        std::string          model;
        std::vector<Message> messages;
        bool                 stream = false;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::model>    = "model",
            cpx::field<&Request::messages> = "messages",
            cpx::field<&Request::stream>   = "stream,skipmissing,omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct Response {
        std::string         id, object, model;
        int64_t             created = 0;
        std::vector<Choice> choices;
        Usage               usage;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Response::id>      = "id",
            cpx::field<&Response::model>   = "model",
            cpx::field<&Response::choices> = "choices",
            cpx::field<&Response::object>  = "object,skipmissing,omitempty",
            cpx::field<&Response::created> = "created,skipmissing,omitempty",
            cpx::field<&Response::usage>   = "usage,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Response>);

    CPX_EXPORT struct ErrorResponse {
        Error error;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ErrorResponse::error> = "error",
        };
    };
    static_assert(std::is_default_constructible_v<ErrorResponse>);
} // namespace cpx::genai::openai::chat_completions


namespace cpx::genai::openai::responses {
    CPX_EXPORT struct Request {
        std::string                                     model;
        std::variant<std::string, std::vector<Message>> input;
        bool                                            stream = false;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::model>  = "model",
            cpx::field<&Request::input>  = "input",
            cpx::field<&Request::stream> = "stream,skipmissing,omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct Response {
        std::string             id;
        std::string             object;
        int64_t                 created_at   = 0;
        int64_t                 completed_at = 0;
        std::string             model;
        std::vector<OutputItem> output;
        Usage                   usage;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Response::id>           = "id",
            cpx::field<&Response::model>        = "model",
            cpx::field<&Response::output>       = "output",
            cpx::field<&Response::object>       = "object,skipmissing,omitempty",
            cpx::field<&Response::created_at>   = "created_at,skipmissing,omitempty",
            cpx::field<&Response::completed_at> = "completed_at,skipmissing,omitempty",
            cpx::field<&Response::usage>        = "usage,skipmissing,omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Response>);

    CPX_EXPORT using ::cpx::genai::openai::chat_completions::ErrorResponse;
} // namespace cpx::genai::openai::responses

namespace cpx {
    CPX_EXPORT namespace openai = cpx::genai::openai;
}

#endif
