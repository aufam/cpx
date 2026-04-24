#ifndef CPX_GENAI_ANTHROPIC_MESSAGES_H
#define CPX_GENAI_ANTHROPIC_MESSAGES_H

#include <cpx/tag.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <optional>


namespace cpx::genai::anthropic {
    struct Error {
        Tag<std::string> type    = "json:`type`";
        Tag<std::string> message = "json:`message`";
    };

    struct Delta {
        Tag<std::string> type = "json:`type`";
        Tag<std::string> text = "json:`text,skipmissing`";
    };

    struct MessagesResponse {
        Tag<std::string>          type    = "json:`type`";
        Tag<std::vector<Delta>>   content = "json:`content,skipmissing`";
        Tag<std::string>          role    = "json:`role,skipmissing`";
        Tag<std::optional<Error>> error   = "json:`error`";


        const std::string &get_text() const & {
            if (error().has_value())
                throw std::runtime_error("Anthropic API error: " + error()->message());
            return content().at(0).text();
        }

        std::string get_text() && {
            if (error().has_value())
                throw std::runtime_error("Anthropic API error: " + error()->message());
            return std::move(content().at(0).text());
        }
    };

    struct MessagesStreamResponse {
        Tag<std::string>          type  = "json:`type`";
        Tag<Delta>                delta = "json:`delta,skipmissing`";
        Tag<std::optional<Error>> error = "json:`error`";

        const std::string &get_text() const & {
            if (error().has_value())
                throw std::runtime_error("Anthropic API error: " + error()->message());
            return delta().text();
        }

        std::string get_text() && {
            if (error().has_value())
                throw std::runtime_error("Anthropic API error: " + error()->message());
            return std::move(delta().text());
        }
    };
} // namespace cpx::genai::anthropic

#endif
