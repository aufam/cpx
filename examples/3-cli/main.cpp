#include <magic_enum/magic_enum.hpp>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

// public API
struct App {
    std::string my_name;
    bool        verbose = false;

    enum LogLevel { Debug, Info, Warn };
    LogLevel log_level = Info;

    std::timespec delay{};

    struct Greet {
        void greet(const std::string &name) {
            fmt::println("Hello from greet {}", name);
        }
    };
    std::optional<Greet> greet;

    struct Add {
        int num1;
        int num2;
    };
    Add add;
};

template <>
struct cpx::Reflect<App> //
    : Fields<cpx::Reflect<App>, &App::my_name, &App::verbose, &App::log_level, &App::delay, &App::greet, &App::add> {
    static constexpr TagInfo my_name   = "name,      short=n,     help=Name to greet,        env=USER";
    static constexpr TagInfo verbose   = "verbose,   short=v,     help=Enable verbose output";
    static constexpr TagInfo log_level = "log-level,              help=Log level,            skipmissing";
    static constexpr TagInfo delay     = "delay,                  help=specify delay,        skipmissing";
    static constexpr TagInfo greet     = "greet,                  help=Greet the name";
    static constexpr TagInfo add       = "add,                    help=Add two numbers";

    static constexpr tags_type tags() {
        return std::tie(my_name, verbose, log_level, delay, greet, add);
    }
};

template <>
struct cpx::Reflect<App::Greet> : Fields<cpx::Reflect<App::Greet>> {};

template <>
struct cpx::Reflect<App::Add> : Fields<cpx::Reflect<App::Add>, &App::Add::num1, &App::Add::num2> {
    static constexpr TagInfo num1 = "num1, positional, help=First number";
    static constexpr TagInfo num2 = "num2, positional, help=Second number";

    static constexpr tags_type tags() {
        return std::tie(num1, num2);
    }
};

int main(int argc, char **argv) {
    auto [app, subcommands] = cpx::cli11::parse_with_subcommands<App>("cli example", argc, argv);

    if (app.verbose)
        fmt::println("app = {}", app);

    // detect subcommand using parsed subcommands tree
    if (subcommands.empty()) {
        fmt::println("{}: You are {}", app.log_level, app.my_name);
    } else if (const std::string &sub = subcommands.front(); sub == "add") {
        auto [a, b] = std::make_tuple(app.add.num1, app.add.num2);
        fmt::println("{}: {} + {} = {}", app.log_level, a, b, a + b);
    }

    // detect subcommand with optional
    if (app.greet.has_value())
        app.greet->greet(app.my_name);
}
