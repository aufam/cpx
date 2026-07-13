#include <httplib.h>
#include <cpx/genai/openai.h>
#include <cpx/json/yy_json.h>
#include <cpx/fmt.h>

int main() {
    cpx::openai::ChatCompletionsRequest req;
    req.messages = {
        {"system", "you are a helpful assistant"},
        {"user",   "hello world"                },
    };
    req.model = "qwen2.5:0.5b";

    httplib::Client cli("http://localhost:11434");

    auto hres = cli.Post("/v1/chat/completions", cpx::yy_json::dump(req), "application/json");
    if (!hres)
        throw std::runtime_error("Request failed");

    auto &body = hres->body;
    if (hres->status != 200)
        throw std::runtime_error(body);

    auto res = cpx::yy_json::parse<cpx::openai::ChatCompletionsResponse>(body);
    fmt::println("{}", res.choices.at(0).message.get_text());
}
