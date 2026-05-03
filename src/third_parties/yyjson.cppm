module;

#define yyjson_api_inline inline
#include <cpx/json/yy_json.h>

export module cpx.yyjson;
import cpx;
export import cpx.serde;
export import cpx.json;

export {
    using ::yyjson_doc;
    using ::yyjson_mut_doc;
    using ::yyjson_mut_val;
    using ::yyjson_val;

    template <typename C, typename CT, typename A>
    struct cpx::serde::Parse<yyjson_doc, std::basic_string<C, CT, A>>;

    template <typename C, typename CT, typename A>
    struct cpx::serde::Dump<yyjson_mut_doc, std::basic_string<C, CT, A>>;

    template <>
    struct cpx::serde::Serialize<yyjson_mut_val, bool>;

    template <>
    struct cpx::serde::Deserialize<yyjson_val, bool>;

    template <typename T>
    struct cpx::serde::Serialize<
        yyjson_mut_val,
        T,
        std::enable_if_t<std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        yyjson_val,
        T,
        std::enable_if_t<std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>>>;

    template <typename T>
    struct cpx::serde::Serialize<yyjson_mut_val, T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Deserialize<yyjson_val, T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, bool>>>;

    template <typename T>
    struct cpx::serde::Serialize<yyjson_mut_val, T, std::enable_if_t<std::is_floating_point_v<T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<yyjson_val, T, std::enable_if_t<std::is_floating_point_v<T>>>;

    template <typename C, typename CT>
    struct cpx::serde::Serialize<yyjson_mut_val, std::basic_string_view<C, CT>>;

    template <typename C, typename CT, typename A>
    struct cpx::serde::Serialize<yyjson_mut_val, std::basic_string<C, CT, A>>;

    template <typename C, typename CT, typename A>
    struct cpx::serde::Deserialize<yyjson_val, std::basic_string<C, CT, A>>;

    template <typename T>
    struct cpx::serde::
        Serialize<yyjson_mut_val, std::optional<T>, std::enable_if_t<cpx::serde::is_serializable_v<yyjson_mut_val, T>>>;

    template <typename T>
    struct cpx::serde::Deserialize<
        yyjson_val,
        std::optional<T>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<yyjson_val, T>>>;

    template <typename T, size_t N>
    struct cpx::serde::
        Serialize<yyjson_mut_val, std::array<T, N>, std::enable_if_t<cpx::serde::is_serializable_v<yyjson_mut_val, T>>>;

    template <typename T, size_t N>
    struct cpx::serde::
        Deserialize<yyjson_val, std::array<T, N>, std::enable_if_t<cpx::serde::is_deserializable_v<yyjson_val, T>>>;

    template <typename T, typename A>
    struct cpx::serde::
        Serialize<yyjson_mut_val, std::vector<T, A>, std::enable_if_t<cpx::serde::is_serializable_v<yyjson_mut_val, T>>>;

    template <typename T, typename A>
    struct cpx::serde::Deserialize<
        yyjson_val,
        std::vector<T, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<yyjson_val, T>>>;

    template <typename... Ts>
    struct cpx::serde::Serialize<yyjson_mut_val, std::tuple<Ts...>>;

    template <typename... Ts>
    struct cpx::serde::Deserialize<yyjson_val, std::tuple<Ts...>>;

    template <typename... T>
    struct cpx::serde::Serialize<
        yyjson_mut_val,
        std::variant<T...>,
        std::enable_if_t<(cpx::serde::is_serializable_v<yyjson_mut_val, T> && ...)>>;

    template <typename... T>
    struct cpx::serde::Deserialize<
        yyjson_val,
        std::variant<T...>,
        std::enable_if_t<((std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<yyjson_val, T>) && ...)>>;

    template <typename C, typename CT, typename CA, typename T, typename H, typename P, typename A>
    struct cpx::serde::Serialize<
        yyjson_mut_val,
        std::unordered_map<std::basic_string<C, CT, CA>, T, H, P, A>,
        std::enable_if_t<cpx::serde::is_serializable_v<yyjson_mut_val, T>>>;

    template <typename C, typename CT, typename CA, typename T, typename H, typename P, typename A>
    struct cpx::serde::Deserialize<
        yyjson_val,
        std::unordered_map<std::basic_string<C, CT, CA>, T, H, P, A>,
        std::enable_if_t<std::is_default_constructible_v<T> && cpx::serde::is_deserializable_v<yyjson_val, T>>>;

    template <>
    struct cpx::serde::Serialize<yyjson_mut_val, std::tm>;

    template <>
    struct cpx::serde::Deserialize<yyjson_val, std::tm>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct cpx::serde::Serialize<yyjson_mut_val, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;

    template <typename S>
    struct cpx::serde::Deserialize<yyjson_val, S, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct cpx::serde::Serialize<yyjson_mut_val, S, std::enable_if_t<std::is_enum_v<S>>>;

    template <typename S>
    struct cpx::serde::Deserialize<yyjson_val, S, std::enable_if_t<std::is_enum_v<S>>>;
#endif
}

export namespace cpx::json::yy_json {
    using ::cpx::json::yy_json::Deserialize;
    using ::cpx::json::yy_json::dump;
    using ::cpx::json::yy_json::Dump;
    using ::cpx::json::yy_json::parse;
    using ::cpx::json::yy_json::Parse;
    using ::cpx::json::yy_json::parse_from_file;
    using ::cpx::json::yy_json::Serialize;
} // namespace cpx::json::yy_json

export namespace cpx {
    namespace yyjson = ::cpx::json::yy_json;
}
