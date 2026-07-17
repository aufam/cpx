#include <httplib.h>
#include <cpx/genai/anthropic.h>
#include <cpx/json/yy_json.h>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

struct Cli {
    std::string              host  = "https://api.anthropic.com";
    std::string              model = "claude-opus-4-6";
    std::string              api_key;
    std::vector<std::string> system;
    std::vector<std::string> user;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Cli::user>    = "user,positional",
        cpx::field<&Cli::system>  = "system",
        cpx::field<&Cli::host>    = "host,skipmissing",
        cpx::field<&Cli::model>   = "model,skipmissing",
        cpx::field<&Cli::api_key> = "api-key,env=ANTHROPIC_API_KEY",
    };
};

int main(int argc, char **argv) {
    Cli cli;
    cpx::cli11::parse("openai example", argc, argv, cli);

    using cpx::anthropic::messages::ErrorResponse;
    using cpx::anthropic::messages::Request;
    using cpx::anthropic::messages::Response;

    Request req;
    req.model = cli.model;

    req.messages.reserve(cli.system.size() + cli.user.size());
    for (auto &text : cli.system) {
        if (text.size() && text[0] == '@') {
            std::ifstream ifs(text.substr(1));
            if (!ifs) {
                fmt::println("cannot open {:?}", text.substr(0));
                exit(1);
            }
            text = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        req.messages.push_back({"system", std::move(text)});
    }
    for (auto &text : cli.user) {
        if (text.size() && text[0] == '@') {
            std::ifstream ifs(text.substr(1));
            if (!ifs) {
                fmt::println("cannot open {:?}", text.substr(0));
                exit(1);
            }
            text = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        }
        req.messages.push_back({"user", std::move(text)});
    }

    if (cli.user.empty()) {
        fmt::print(stderr, "user: ");
        std::string text;
        std::cin >> text;
        req.messages.push_back({"user", std::move(text)});
    }

    auto client = httplib::Client(cli.host);
    auto hres   = client.Post(
        "/v1/messages",
        {
            {"x-api-key", cli.api_key},
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
    fmt::println("{}", res.get_text());
}
