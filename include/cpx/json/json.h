#ifndef CPX_JSON_JSON_H
#define CPX_JSON_JSON_H

#include <cpx/tag_info.h>
#include <cpx/serde/serialize.h>
#include <cpx/serde/deserialize.h>
#include <cpx/time.h>

namespace cpx::json {
    struct JsonGeneric {};

    template <typename From>
    using Serialize = ::cpx::serde::Serialize<JsonGeneric, From>;

    template <typename To>
    using Deserialize = ::cpx::serde::Deserialize<JsonGeneric, To>;

    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        if constexpr (::cpx::detail::is_value_and_tag_info_v<T>)
            return std::get<1>(field);
        else
            return ::cpx::get_tag_info(field, "json");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        TagInfoTuple<sizeof...(T)> ts       = {};
        bool                       is_array = sizeof...(T) > 0;

        tuple_for_each(fields, [&](const auto &field, size_t i) {
            TagInfo &t = ts.ts[i] = ::cpx::json::get_tag_info(field);
            is_array &= t.positional;
        });

        ts.is_obj = !is_array;
        return ts;
    }
} // namespace cpx::json


template <typename T>
struct cpx::serde::Serialize<cpx::json::JsonGeneric, T> : std::false_type {};

template <typename T>
struct cpx::serde::Deserialize<cpx::json::JsonGeneric, T> : std::false_type {};

template <>
struct cpx::serde::Serialize<cpx::json::JsonGeneric, std::tm> : std::true_type {
    using From = std::string;
    From from(const std::tm &tm) {
        return cpx::tm_to_string(tm);
    }
};

template <>
struct cpx::serde::Deserialize<cpx::json::JsonGeneric, std::tm> : std::true_type {
    using Into = std::string;
    auto into(std::tm &tm) {
        struct _Into {
            std::tm &tm;
            Into     into{};

            operator Into &() {
                return into;
            }
            ~_Into() {
                tm = cpx::tm_from_string(into);
            }
        } ret{tm};
        return ret;
    }
};

#endif
