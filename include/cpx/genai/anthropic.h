#ifndef CPX_GENAI_ANTHROPIC_H
#define CPX_GENAI_ANTHROPIC_H

#include <cpx/genai/openai_common.h>

namespace cpx::genai::anthropic {
    CPX_EXPORT using ::cpx::genai::openai::TextBlock;
    CPX_EXPORT using ::cpx::genai::openai::ImageBlock;
    CPX_EXPORT using ::cpx::genai::openai::ImageURLBlock;
    CPX_EXPORT using ::cpx::genai::openai::Content;
    CPX_EXPORT using ::cpx::genai::openai::Error;
    CPX_EXPORT using ::cpx::genai::openai::Message;
    CPX_EXPORT using ::cpx::genai::openai::Usage;
} // namespace cpx::genai::anthropic

namespace cpx::genai::anthropic::messages {
    CPX_EXPORT struct Request {
        int                  max_tokens = 1024;
        std::string          model;
        std::vector<Message> messages;
        bool                 stream = false;

        std::variant<std::string, std::vector<Content>> system;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Request::max_tokens> = "max_tokens",
            cpx::field<&Request::model>      = "model",
            cpx::field<&Request::messages>   = "messages",
            cpx::field<&Request::stream>     = "stream,skipmissing,omitempty",
            cpx::field<&Request::system>     = "system,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Request>);

    CPX_EXPORT struct MessageResponse {
        std::string          id, model, role, stop_reason, type = "message";
        std::vector<Content> content;
        Usage                usage;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&MessageResponse::type>        = "type",
            cpx::field<&MessageResponse::id>          = "id",
            cpx::field<&MessageResponse::model>       = "model",
            cpx::field<&MessageResponse::role>        = "role",
            cpx::field<&MessageResponse::stop_reason> = "stop_reason",
            cpx::field<&MessageResponse::content>     = "content",
            cpx::field<&MessageResponse::usage>       = "usage,skipmissing,omitempty",
        };

        const std::string &get_text() const {
            return std::get<TextBlock>(content.at(0)).text;
        }
    };
    static_assert(std::is_default_constructible_v<MessageResponse>);

    CPX_EXPORT using Response = MessageResponse;

    CPX_EXPORT struct StartEvent {
        MessageResponse message;
        std::string     type = "message_start";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&StartEvent::type>    = "type",
            cpx::field<&StartEvent::message> = "message",
        };
    };
    static_assert(std::is_default_constructible_v<StartEvent>);

    CPX_EXPORT struct BlockStartEvent {
        Content     content_block;
        int         index = 0;
        std::string type  = "content_block_start";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&BlockStartEvent::type>          = "type",
            cpx::field<&BlockStartEvent::index>         = "index",
            cpx::field<&BlockStartEvent::content_block> = "content_block",
        };
    };
    static_assert(std::is_default_constructible_v<BlockStartEvent>);

    CPX_EXPORT struct DeltaEvent {
        struct Delta {
            std::string stop_reason;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Delta::stop_reason> = "stop_reason",
            };
        } delta;

        Usage       usage;
        std::string type = "message_delta";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&DeltaEvent::type>  = "type",
            cpx::field<&DeltaEvent::delta> = "delta",
            cpx::field<&DeltaEvent::usage> = "usage",
        };
    };
    static_assert(std::is_default_constructible_v<DeltaEvent>);

    CPX_EXPORT struct BlockDeltaEvent {
        Content     delta;
        int         index = 0;
        std::string type  = "content_block_delta";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&BlockDeltaEvent::type>  = "type",
            cpx::field<&BlockDeltaEvent::index> = "index",
            cpx::field<&BlockDeltaEvent::delta> = "delta",
        };
    };
    static_assert(std::is_default_constructible_v<BlockDeltaEvent>);

    CPX_EXPORT struct OtherEvent {
        std::string type;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&BlockDeltaEvent::type> = "type",
        };
    };
    static_assert(std::is_default_constructible_v<OtherEvent>);

    CPX_EXPORT using StreamResponse = std::variant<StartEvent, DeltaEvent, BlockStartEvent, BlockDeltaEvent, OtherEvent>;
    static_assert(std::is_default_constructible_v<StreamResponse>);

    CPX_EXPORT struct ErrorResponse {
        Error error;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ErrorResponse::error> = "error",
        };
    };
    static_assert(std::is_default_constructible_v<ErrorResponse>);
} // namespace cpx::genai::anthropic::messages


namespace cpx {
    CPX_EXPORT namespace anthropic = cpx::genai::anthropic;
}

#endif
