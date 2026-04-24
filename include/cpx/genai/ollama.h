#ifndef CPX_GENAI_OLLAMA_H
#define CPX_GENAI_OLLAMA_H

#ifndef CPPHTTPLIB_HTTPLIB_H
#    include <httplib.h>
#endif

#include <cpx/genai/ollama_generate.h>
#include <cpx/json/yy_json.h>

namespace cpx::genai::ollama {
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

        GenerateContentResponse generate_content(const GenerateContentRequest &req) {
            std::string path = "/api/generate";

            auto res = cli.Post(path, json::yy_json::dump(req), "application/json");
            if (!res)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(res.error()));

            return json::yy_json::parse<GenerateContentResponse>(res->body);
        }

        httplib::stream::Result generate_content_stream(const GenerateContentRequest &req) {
            std::string path = "/api/generate";

            auto ret = httplib::stream::Post(cli, path, json::yy_json::dump(req), "application/json");
            if (!ret)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(ret.error()));
            if (ret.get_header_value("Content-Type") == "application/json") {
                auto body = ret.read_all();
                auto res  = json::yy_json::parse<GenerateContentResponse>(body);
                if (!res.error().empty())
                    throw std::runtime_error("Ollama generate content stream request failed: " + res.error());
            }
            return ret;
        }

        template <typename Message, typename Request>
        ChatCompletionsResponse<Message> create_chat_completions(const Request &req) {
            std::string path = "/api/chat";

            auto res = cli.Post(path, json::yy_json::dump(req), "application/json");
            if (!res)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(res.error()));

            return json::yy_json::parse<ChatCompletionsResponse<Message>>(res->body);
        }

        template <typename Message, typename Request>
        httplib::stream::Result create_chat_completions_stream(const Request &req) {
            std::string path = "/api/chat";

            auto ret = httplib::stream::Post(cli, path, json::yy_json::dump(req), "application/json");
            if (!ret)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(ret.error()));
            if (ret.get_header_value("Content-Type") == "application/json") {
                auto body = ret.read_all();
                auto res  = json::yy_json::parse<ChatCompletionsResponse<Message>>(body);
                if (!res.error().empty())
                    throw std::runtime_error("Ollama chat completions stream request failed: " + res.error());
            }
            return ret;
        }

        // TODO: other ollama API?
    };
} // namespace cpx::genai::ollama

#endif
