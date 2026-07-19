#include <httplib.h>
#include <cpx/genai/gemini.h>
#include <cpx/json/yy_json.h>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

struct Cli {
    std::string              host  = "https://generativelanguage.googleapis.com";
    std::string              model = "gemini-2.5-flash-lite";
    std::string              api_key;
    std::vector<std::string> system;
    std::vector<std::string> user;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Cli::user>    = "user,positional",
        cpx::field<&Cli::system>  = "system",
        cpx::field<&Cli::host>    = "host,skipmissing",
        cpx::field<&Cli::model>   = "model,skipmissing",
        cpx::field<&Cli::api_key> = "api-key,env=GEMINI_API_KEY",
    };
};

int main(int argc, char **argv) {
    Cli cli;
    cpx::cli11::parse("gemini example", argc, argv, cli);

    using cpx::gemini::generate_content::ErrorResponse;
    using cpx::gemini::generate_content::Request;
    using cpx::gemini::generate_content::Response;

    Request req;
    req.contents.reserve(cli.system.size() + cli.user.size());
    for (auto &text : cli.system) {
        req.contents.push_back({
            .role  = "model",
            .parts = {{.text = std::move(text)}},
        });
    }
    for (auto &text : cli.user) {
        req.contents.push_back({
            .role  = "user",
            .parts = {{.text = std::move(text)}},
        });
    }

    if (cli.user.empty()) {
        fmt::print(stderr, "user: ");
        std::string text;
        std::getline(std::cin, text);
        req.contents.push_back({
            .role  = "user",
            .parts = {{.text = std::move(text)}},
        });
    }

    auto client = httplib::Client(cli.host);
    auto hres   = client.Post(
        "/v1/models/" + cli.model + ":generateContent?key=" + cli.api_key, //
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
    fmt::println("{}", res.candidates.at(0).content.parts.at(0).text);
}
