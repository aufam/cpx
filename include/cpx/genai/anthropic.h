#ifndef CPX_GENAI_ANTHROPIC_H
#define CPX_GENAI_ANTHROPIC_H

#ifndef CPPHTTPLIB_HTTPLIB_H
#    include <httplib.h>
#endif

#include <cpx/genai/anthropic_messages.h>
#include <cpx/json/yy_json.h>
#include <httplib.h>

namespace cpx::genai::anthropic {
    class Client {
    protected:
        httplib::Client cli;

    public:
        Client(const std::string &base_url, const std::string &api_key)
            : cli(base_url) {
            if (!api_key.empty())
                cli.set_default_headers({
                    {"Authorization", "Bearer " + api_key}
                });
        }

        template <typename Request>
        MessagesResponse generate_messages(const Request &req, const std::string &version = "v1") {
            std::string path = "/" + version + "/messages";

            auto res = cli.Post(path, json::yy_json::dump(req), "application/json");
            if (!res)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(res.error()));

            return json::yy_json::parse<MessagesResponse>(res->body);
        }

        template <typename Request>
        httplib::stream::Result generate_messages_stream(const Request &req, const std::string &version = "v1") {
            std::string path = "/" + version + "/chat/completions";

            auto ret = httplib::stream::Post(cli, path, json::yy_json::dump(req), "application/json");
            if (!ret)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(ret.error()));
            if (ret.get_header_value("Content-Type") == "application/json") {
                auto body = ret.read_all();
                auto res  = json::yy_json::parse<MessagesResponse>(body);
                if (res.error().has_value())
                    throw std::runtime_error("Gemini stream request failed: " + res.error().value().message());
            }
            return ret;
        }

        // TODO: other anthropic features?
    };
} // namespace cpx::genai::anthropic

#endif
