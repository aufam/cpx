#include <httplib.h>
#include <cpx/genai/openai.h>
#include <cpx/json/yy_json.h>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

struct Cli {
    std::string              host  = "http://localhost:11434";
    std::string              model = "qwen2.5:0.5b";
    std::string              api_key;
    std::vector<std::string> system;
    std::vector<std::string> user;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Cli::user>    = "user,positional",
        cpx::field<&Cli::system>  = "system",
        cpx::field<&Cli::host>    = "host,skipmissing",
        cpx::field<&Cli::model>   = "model,skipmissing",
        cpx::field<&Cli::api_key> = "api-key,env=OPENAI_API_KEY",
    };
};

int main(int argc, char **argv) {
    Cli cli;
    cpx::cli11::parse("openai example", argc, argv, cli);

    using cpx::openai::chat_completions::ErrorResponse;
    using cpx::openai::chat_completions::Request;
    using cpx::openai::chat_completions::Response;

    Request req;
    req.model = cli.model;

    req.messages.reserve(cli.system.size() + cli.user.size());
    for (auto &text : cli.system) {
        req.messages.push_back({"system", std::move(text)});
    }
    for (auto &text : cli.user) {
        req.messages.push_back({"user", std::move(text)});
    }

    if (cli.user.empty()) {
        fmt::print(stderr, "user: ");
        std::string text;
        std::getline(std::cin, text);
        req.messages.push_back({"user", std::move(text)});
    }

    auto client = httplib::Client(cli.host);
    auto hres   = client.Post(
        "/v1/chat/completions",
        {
            {"Authorization", "Bearer " + cli.api_key},
    },
        cpx::yy_json::dump(req),
        "application/json"
    );

    if (!hres) {
        fmt::println("Request failed");
        exit(1);
    }

    auto &body = hres->body;
    if (hres->status != 200) {
        auto res = cpx::yy_json::parse<ErrorResponse>(body);
        fmt::println("error: {}", res.error.message);
        exit(1);
    }

    auto res = cpx::yy_json::parse<Response>(body);
    fmt::println("{}", res.choices.at(0).message.get_text());
}
