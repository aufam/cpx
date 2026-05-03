module;

#include <cpx/json/nlohmann_json.h>

export module cpx.nlohmann.json;
import cpx;
export import cpx.serde;
export import cpx.json;
export import nlohmann.json;

export {
    template <typename T>
    struct cpx::serde::Serialize<nlohmann::json, T, std::enable_if_t<std::is_convertible_v<T, nlohmann::json>>>;

    template <typename T>
    struct cpx::serde::
        Deserialize<nlohmann::json, T, std::void_t<decltype(std::declval<const nlohmann::json &>().get_to(std::declval<T &>()))>>;

    template <>
    struct cpx::serde::Parse<nlohmann::json, std::string>;

    template <>
    struct cpx::serde::Parse<nlohmann::json, std::istream>;

    template <>
    struct cpx::serde::Parse<nlohmann::json, std::FILE *>;

    template <>
    struct cpx::serde::Dump<nlohmann::json, std::string>;

    template <typename T>
    struct nlohmann::adl_serializer<
        std::optional<T>,
        std::enable_if_t<
            std::is_convertible_v<T, nlohmann::json>,
            std::void_t<decltype(std::declval<const nlohmann::json &>().get<T>())>>>;

    template <typename... Ts>
    struct nlohmann::adl_serializer<std::tuple<Ts...>>;

    template <typename... T>
    struct nlohmann::adl_serializer<
        std::variant<T...>,
        std::enable_if_t<
            ((std::is_convertible_v<T, nlohmann::json> && cpx::serde::is_deserializable_v<nlohmann::json, T>) && ...)>>;

    template <>
    struct nlohmann::adl_serializer<std::tm>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct nlohmann::adl_serializer<S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct nlohmann::adl_serializer<S, std::enable_if_t<std::is_enum_v<S>>>;
#endif
} // namespace nlohmann

export namespace cpx::json::nlohmann_json {
    using ::cpx::json::nlohmann_json::Deserialize;
    using ::cpx::json::nlohmann_json::dump;
    using ::cpx::json::nlohmann_json::Dump;
    using ::cpx::json::nlohmann_json::parse;
    using ::cpx::json::nlohmann_json::Parse;
    using ::cpx::json::nlohmann_json::Serialize;
} // namespace cpx::json::nlohmann_json

export namespace cpx {
    namespace nlohmann_json = ::cpx::json::nlohmann_json;
}
