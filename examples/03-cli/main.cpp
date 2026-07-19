#include <magic_enum/magic_enum.hpp>
#include <cpx/fmt.h>
#include <cpx/cli/cli11.h>

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

    static constexpr auto __field_tags__ = std::make_tuple(
        cpx::field<&App::my_name>   = "name,      short=n,     help=Name to greet,        env=USER   ",
        cpx::field<&App::verbose>   = "verbose,   short=v,     help=Enable verbose output            ",
        cpx::field<&App::log_level> = "log-level,              help=Log level,            skipmissing",
        cpx::field<&App::delay>     = "delay,                  help=specify delay,        skipmissing",
        cpx::field<&App::greet>     = "greet,                  help=Greet the name                   ",
        cpx::field<&App::add>       = "add,                    help=Add two numbers                  "
    );
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
