module;

#include <cpx/cli/cli11.h>

export module cpx.cli11;
import cpx;
export import cpx.cli;
export import cli11;

export {
    template <>
    struct cpx::serde::Parse<CLI::App, std::pair<int, char **>>;

    template <>
    struct cpx::serde::Deserialize<CLI::App, bool>;

    template <typename T>
    struct cpx::serde::Deserialize<
        CLI::App,
        T,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        CLI::App,
        std::vector<T>,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        CLI::App,
        std::optional<T>,
        std::enable_if_t<(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>>>;

    template <typename... T>
    struct cpx::serde::Deserialize<
        CLI::App,
        std::variant<T...>,
        std::enable_if_t<(((std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) && !std::is_same_v<T, bool>) && ...)>>;

    template <typename... Ts>
    struct cpx::serde::Deserialize<CLI::App, std::tuple<Ts...>>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct cpx::serde::Deserialize<CLI::App, S, std::enable_if_t<std::is_aggregate_v<S>>>;
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct cpx::serde::Deserialize<CLI::App, S, std::enable_if_t<std::is_enum_v<S>>>;

    template <typename S>
    struct cpx::serde::Deserialize<CLI::App, std::optional<S>, std::enable_if_t<std::is_enum_v<S>>>;
#endif
}

export namespace cpx::cli::cli11 {
    using ::cpx::cli::cli11::Deserialize;
    using ::cpx::cli::cli11::is_deserializable;
    using ::cpx::cli::cli11::parse;
    using ::cpx::cli::cli11::Parse;
    using ::cpx::cli::cli11::parse_with_subcommands;
} // namespace cpx::cli::cli11

export namespace cpx {
    namespace cli11 = ::cpx::cli::cli11;
}
