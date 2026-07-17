#ifndef CPX_GENAI_OLLAMA_H
#define CPX_GENAI_OLLAMA_H

#include <cpx/genai/openai_common.h>
#include <cpx/time.h>

namespace cpx::genai::ollama {
    CPX_EXPORT using ::cpx::genai::openai::Message;

    CPX_EXPORT struct Error {
        std::string error;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Error::error> = "error",
        };
    };
    static_assert(std::is_default_constructible_v<Error>);
} // namespace cpx::genai::ollama

namespace cpx::genai::ollama::chat {
    CPX_EXPORT struct Request {
        std::string          model;
        std::vector<Message> messages;
        bool                 stream = false;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::messages> = "messages",
            cpx::field<&Request::model>    = "model",
            cpx::field<&Request::stream>   = "stream,skipmissing,omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct Response {
        Message       message;
        std::string   model;
        std::timespec created_at = {};

        bool             done = true;
        std::string      done_reason;
        std::vector<int> context;
        int64_t          total_duration = 0, load_duration = 0;
        int              prompt_eval_count = 0, eval_count = 0;
        int64_t          prompt_eval_duration = 0, eval_duration = 0;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Response::message>              = "message",
            cpx::field<&Response::model>                = "model",
            cpx::field<&Response::created_at>           = "created_at",
            cpx::field<&Response::done>                 = "done,skipmissing,omitempty",
            cpx::field<&Response::done_reason>          = "done_reason,skipmissing,omitempty",
            cpx::field<&Response::context>              = "context,skipmissing,omitempty",
            cpx::field<&Response::total_duration>       = "total_duration,skipmissing,omitempty",
            cpx::field<&Response::load_duration>        = "load_duration,skipmissing,omitempty",
            cpx::field<&Response::prompt_eval_count>    = "prompt_eval_count,skipmissing,omitempty",
            cpx::field<&Response::eval_count>           = "eval_count,skipmissing,omitempty",
            cpx::field<&Response::prompt_eval_duration> = "prompt_eval_duration,skipmissing,omitempty",
            cpx::field<&Response::eval_duration>        = "eval_duration,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Response>);
} // namespace cpx::genai::ollama::chat

namespace cpx::genai::ollama::generate {
    CPX_EXPORT struct Request {
        std::string prompt, model;
        bool        stream = false;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::prompt> = "prompt",
            cpx::field<&Request::model>  = "model",
            cpx::field<&Request::stream> = "stream,skipmissing,omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct Response {
        std::string   model, response;
        std::timespec created_at;

        bool             done = true;
        std::string      done_reason;
        std::vector<int> context;
        int64_t          total_duration = 0, load_duration = 0;
        int              prompt_eval_count = 0, eval_count = 0;
        int64_t          prompt_eval_duration = 0, eval_duration = 0;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Response::response>             = "response",
            cpx::field<&Response::model>                = "model",
            cpx::field<&Response::created_at>           = "created_at",
            cpx::field<&Response::done>                 = "done,skipmissing,omitempty",
            cpx::field<&Response::done_reason>          = "done_reason,skipmissing,omitempty",
            cpx::field<&Response::context>              = "context,skipmissing,omitempty",
            cpx::field<&Response::total_duration>       = "total_duration,skipmissing,omitempty",
            cpx::field<&Response::load_duration>        = "load_duration,skipmissing,omitempty",
            cpx::field<&Response::prompt_eval_count>    = "prompt_eval_count,skipmissing,omitempty",
            cpx::field<&Response::eval_count>           = "eval_count,skipmissing,omitempty",
            cpx::field<&Response::prompt_eval_duration> = "prompt_eval_duration,skipmissing,omitempty",
            cpx::field<&Response::eval_duration>        = "eval_duration,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Response>);
} // namespace cpx::genai::ollama::generate

namespace cpx {
    CPX_EXPORT namespace ollama = cpx::genai::ollama;
}

#endif
