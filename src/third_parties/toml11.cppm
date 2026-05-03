module;

#include <cpx/toml/toruniina_toml.h>

export module cpx.toml11;
import cpx;
export import cpx.serde;
export import cpx.toml;

export {
    template <>
    struct cpx::serde::Dump<__toml11::table, std::string>;

    template <>
    struct cpx::serde::Parse<__toml11::table, std::string>;

    template <>
    struct cpx::serde::Serialize<__toml11::value, bool>;

    template <>
    struct cpx::serde::Deserialize<__toml11::value, bool>;

    template <typename T>
    struct cpx::serde::Serialize<__toml11::value, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Deserialize<__toml11::value, T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Serialize<__toml11::value, T, std::enable_if_t<std::is_floating_point_v<T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<__toml11::value, T, std::enable_if_t<std::is_floating_point_v<T>>>;

    template <>
    struct cpx::serde::Serialize<__toml11::value, std::string>;

    template <>
    struct cpx::serde::Serialize<__toml11::value, std::string_view>;

    template <>
    struct cpx::serde::Deserialize<__toml11::value, std::string>;

    template <typename T>
    struct cpx::serde::Serialize<
        __toml11::value,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_serializable_v<__toml11::value, T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        __toml11::value,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__toml11::value, T>>>;

    template <typename T, size_t N>
    struct cpx::serde::
        Serialize<__toml11::value, std::array<T, N>, std::enable_if_t<cpx::serde::is_serializable_v<__toml11::value, T>>>;

    template <typename T, size_t N>
    struct cpx::serde::
        Deserialize<__toml11::value, std::array<T, N>, std::enable_if_t<cpx::serde::is_deserializable_v<__toml11::value, T>>>;

    template <typename T>
    struct cpx::serde::
        Serialize<__toml11::value, std::vector<T>, std::enable_if_t<cpx::serde::is_serializable_v<__toml11::value, T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        __toml11::value,
        std::vector<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__toml11::value, T>>>;

    template <typename... Ts>
    struct cpx::serde::Serialize<__toml11::value, std::tuple<Ts...>>;

    template <typename... Ts>
    struct cpx::serde::Deserialize<__toml11::value, std::tuple<Ts...>>;

    template <typename... T>
    struct cpx::serde::Serialize<__toml11::value, std::variant<T...>>;

    template <typename... T>
    struct cpx::serde::
        Deserialize<__toml11::value, std::variant<T...>, std::enable_if_t<(std::is_default_constructible_v<T> && ...)>>;

    template <typename T>
    struct cpx::serde::Serialize<
        __toml11::value,
        std::unordered_map<std::string, T>,
        std::enable_if_t<cpx::serde::is_serializable_v<__toml11::value, T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        __toml11::value,
        std::unordered_map<std::string, T>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<__toml11::value, T>>>;

    template <>
    struct cpx::serde::Serialize<__toml11::value, std::tm>;

    template <>
    struct cpx::serde::Deserialize<__toml11::value, std::tm>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct cpx::serde::Serialize<__toml11::value, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;

    template <typename S>
    struct cpx::serde::Deserialize<__toml11::value, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct cpx::serde::Serialize<__toml11::value, S, std::enable_if_t<std::is_enum_v<S>>>;

    template <typename S>
    struct cpx::serde::Deserialize<__toml11::value, S, std::enable_if_t<std::is_enum_v<S>>>;
#endif
} // namespace cpx::serde

export namespace cpx::toml::toruniina_toml {
    using spec = __toml11::spec;

    using ::cpx::toml::toruniina_toml::Deserialize;
    using ::cpx::toml::toruniina_toml::dump;
    using ::cpx::toml::toruniina_toml::Dump;
    using ::cpx::toml::toruniina_toml::parse;
    using ::cpx::toml::toruniina_toml::Parse;
    using ::cpx::toml::toruniina_toml::parse_from_file;
    using ::cpx::toml::toruniina_toml::Serialize;
} // namespace cpx::toml::toruniina_toml

export namespace cpx {
    namespace toml11 = ::cpx::toml::toruniina_toml;
}
