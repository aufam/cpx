#include <httplib.h>
#include <cpx/genai/ollama.h>
#include <cpx/json/yy_json.h>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

struct Generate {
    std::string prompt;
    std::string model  = "qwen2.5:0.5b";
    bool        stream = false;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Generate::prompt> = "prompt,positional,skipmissing",
        cpx::field<&Generate::stream> = "stream",
        cpx::field<&Generate::model>  = "model,skipmissing",
    };
};

struct Chat {
    std::vector<std::string> system;
    std::vector<std::string> user;

    std::string model  = "qwen2.5:0.5b";
    bool        stream = false;

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Chat::user>   = "user,positional",
        cpx::field<&Chat::system> = "system",
        cpx::field<&Chat::stream> = "stream",
        cpx::field<&Chat::model>  = "model,skipmissing",
    };
};

struct Cli {
    std::optional<Chat>     chat;
    std::optional<Generate> generate;
    std::string             host = "http://localhost:11434";

    static constexpr std::tuple __field_tags__ = {
        cpx::field<&Cli::chat>     = "chat,oneof=chat|generate",
        cpx::field<&Cli::generate> = "generate,oneof=chat|generate",
        cpx::field<&Cli::host>     = "host,skipmissing",
    };
};

int main(int argc, char **argv) {
    Cli cli;
    cpx::cli11::parse("ollama example", argc, argv, cli);

    std::string req_body, api;
    bool        is_chat_api;

    if (auto &chat = cli.chat; chat.has_value()) {
        cpx::ollama::chat::Request req;
        req.model  = chat->model;
        req.stream = chat->stream;

        req.messages.reserve(chat->system.size() + chat->user.size());
        for (auto &text : chat->system) {
            req.messages.push_back({"system", std::move(text)});
        }
        for (auto &text : chat->user) {
            req.messages.push_back({"user", std::move(text)});
        }

        if (chat->user.empty()) {
            fmt::print(stderr, "user: ");
            std::string text;
            std::getline(std::cin, text);
            req.messages.push_back({"user", std::move(text)});
        }

        req_body    = cpx::yy_json::dump(req);
        api         = "/api/chat";
        is_chat_api = true;
    } else if (auto &generate = cli.generate; generate.has_value()) {
        cpx::ollama::generate::Request req;
        req.model  = generate->model;
        req.stream = generate->stream;
        req.prompt = std::move(generate->prompt);

        if (auto &text = req.prompt; text.empty()) {
            fmt::print(stderr, "user: ");
            std::getline(std::cin, text);
        }

        req_body    = cpx::yy_json::dump(req);
        api         = "/api/generate";
        is_chat_api = false;
    } else {
        fmt::println(stderr, "unreachable");
        exit(1);
    }

    auto client = httplib::Client(cli.host);
    auto stream = httplib::stream::Post(client, api, req_body, "application/json");
    if (!stream) {
        fmt::println(stderr, "request failed");
        exit(1);
    }

    if (stream.status() != 200) {
        auto body = stream.read_all();
        auto res  = cpx::yy_json::parse<cpx::ollama::Error>(body);
        fmt::println(stderr, "error: {}", res.error);
        exit(1);
    }

    while (stream.next()) {
        std::string_view body = {stream.data(), stream.size()};

        if (is_chat_api) {
            auto res = cpx::yy_json::parse<cpx::ollama::chat::Response>(body);
            fmt::print("{}", res.message.get_text());
        } else {
            auto res = cpx::yy_json::parse<cpx::ollama::generate::Response>(body);
            fmt::print("{}", res.response);
        }
    }
    fmt::println("");

    return 0;
}
