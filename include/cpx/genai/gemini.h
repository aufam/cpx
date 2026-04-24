#ifndef CPX_GENAI_GEMINI_H
#define CPX_GENAI_GEMINI_H

#ifndef CPPHTTPLIB_HTTPLIB_H
#    include <httplib.h>
#endif

#include <cpx/genai/gemini_generate.h>
#include <cpx/json/yy_json.h>

namespace cpx::genai::gemini {
    class Client {
    protected:
        httplib::Client cli;

    public:
        Client(const std::string &base_url, const std::string &api_key)
            : cli(base_url) {
            if (!api_key.empty())
                cli.set_default_headers({
                    {"x-goog-api-key", api_key}
                });
        }

        GenerateContentResponse
        generate_content(const std::string &model, const GenerateContentRequest &req, const std::string &version = "v1") {
            std::string path = "/" + version + "/models/" + model + ":generateContent";

            auto res = cli.Post(path, json::yy_json::dump(req), "application/json");
            httplib::to_string(res.error());
            if (!res)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(res.error()));

            return json::yy_json::parse<GenerateContentResponse>(res->body);
        }

        httplib::stream::Result
        generate_content_stream(const std::string &model, const GenerateContentRequest &req, const std::string &version = "v1") {
            std::string path = "/" + version + "/models/" + model + ":streamGenerateContent?alt=sse";

            auto ret = httplib::stream::Post(cli, path, json::yy_json::dump(req), "application/json");
            if (!ret)
                throw std::runtime_error("httplib request failed: code=" + httplib::to_string(ret.error()));
            if (ret.get_header_value("Content-Type") == "application/json") {
                auto body = ret.read_all();
                auto res  = json::yy_json::parse<GenerateContentResponse>(body);
                if (res.error().has_value())
                    throw std::runtime_error("Gemini stream request failed: " + res.error().value().message());
            }
            return ret;
        }

        // TODO: other gemini API?
    };
} // namespace cpx::genai::gemini

#endif
