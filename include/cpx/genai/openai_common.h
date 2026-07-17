#ifndef CPX_GENAI_OPENAI_COMMON_H
#define CPX_GENAI_OPENAI_COMMON_H

#include <cpx/tag_info.h>
#include <string>
#include <vector>
#include <variant>

namespace cpx::genai::openai {
    CPX_EXPORT struct TextBlock {
        std::string text, type = "text";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&TextBlock::type> = "type",
            cpx::field<&TextBlock::text> = "text",
        };
    };
    static_assert(std::is_default_constructible_v<TextBlock>);

    CPX_EXPORT struct ImageBlock {
        struct Base64 {
            std::string data, media_type, type;
            Base64()
                : type("Base64") {} // TODO: https://godbolt.org/z/vqTjxv9zW

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&Base64::data>       = "data",
                cpx::field<&Base64::media_type> = "media_type",
                cpx::field<&Base64::type>       = "type",
            };
        };

        struct URL {
            std::string url, type = "url";

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&URL::url>  = "url",
                cpx::field<&URL::type> = "type",
            };
        };

        std::variant<Base64, URL> source;
        std::string               type = "image";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ImageBlock::source> = "source",
            cpx::field<&ImageBlock::type>   = "type",
        };
    };
    static_assert(std::is_default_constructible_v<ImageBlock>);
    static_assert(std::is_default_constructible_v<ImageBlock::Base64>);
    static_assert(std::is_default_constructible_v<ImageBlock::URL>);

    CPX_EXPORT struct ImageURLBlock {
        struct ImageURL {
            std::string url;

            static constexpr std::tuple __field_tags__ = {
                cpx::field<&ImageURL::url> = "url",
            };
        } image_url;

        std::string type = "image_url";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ImageURLBlock::type>      = "type",
            cpx::field<&ImageURLBlock::image_url> = "image_url",
        };
    };
    static_assert(std::is_default_constructible_v<ImageURLBlock>);
    static_assert(std::is_default_constructible_v<ImageURLBlock::ImageURL>);

    CPX_EXPORT struct ThinkingBlock {
        std::string thinking, signature, type = "thinking";

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&ThinkingBlock::type>      = "type",
            cpx::field<&ThinkingBlock::thinking>  = "thinking",
            cpx::field<&ThinkingBlock::signature> = "signature,skipmissing,omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<ThinkingBlock>);

    CPX_EXPORT using Content = std::variant<TextBlock, ImageBlock, ImageURLBlock, ThinkingBlock>; // TODO: other blocks
    static_assert(std::is_default_constructible_v<Content>);

    CPX_EXPORT struct Message {
        std::string                                     role;
        std::variant<std::string, std::vector<Content>> content;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Message::role>    = "role   ,skipmissing,omitempty",
            cpx::field<&Message::content> = "content,skipmissing,omitempty",
        };

        bool empty() const {
            return role.empty() && std::visit([](const auto &c) { return c.empty(); }, content);
        }

        const std::string &get_text() const {
            if (std::holds_alternative<std::string>(content)) {
                return std::get<std::string>(content);
            } else {
                auto &first_content = std::get<std::vector<Content>>(content).at(0);
                return std::get<TextBlock>(first_content).text;
            }
        }
    };
    static_assert(std::is_default_constructible_v<Message>);

    CPX_EXPORT struct Usage {
        int prompt_tokens               = 0;
        int completion_tokens           = 0;
        int input_tokens                = 0;
        int output_tokens               = 0;
        int cache_creation_input_tokens = 0;
        int cache_read_input_tokens     = 0;

        bool empty() const {
            return prompt_tokens == 0 && completion_tokens == 0 && //
                   input_tokens == 0 && output_tokens == 0 &&      //
                   cache_creation_input_tokens == 0 && cache_read_input_tokens == 0;
        }

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Usage::prompt_tokens>               = "prompt_tokens                , skipmissing , omitempty",
            cpx::field<&Usage::completion_tokens>           = "completion_tokens            , skipmissing , omitempty",
            cpx::field<&Usage::input_tokens>                = "input_tokens                 , skipmissing , omitempty",
            cpx::field<&Usage::output_tokens>               = "output_tokens                , skipmissing , omitempty",
            cpx::field<&Usage::cache_creation_input_tokens> = "cache_creation_input_tokens  , skipmissing , omitempty",
            cpx::field<&Usage::cache_read_input_tokens>     = "cache_read_input_tokens      , skipmissing , omitempty",
        };
    };
    static_assert(std::is_default_constructible_v<Usage>);

    CPX_EXPORT struct Error {
        std::string message, type, param, code;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&Error::message> = "message",
            cpx::field<&Error::type>    = "type",
            cpx::field<&Error::param>   = "param    , skipmissing , omitempty",
            cpx::field<&Error::code>    = "code     , skipmissing , omitempty"
        };
    };
    static_assert(std::is_default_constructible_v<Error>);
} // namespace cpx::genai::openai

#endif
