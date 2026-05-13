#ifndef CPX_TOML_TOML_H
#define CPX_TOML_TOML_H

#include <cpx/reflect.h>

namespace cpx::toml {
    template <typename T>
    constexpr TagInfo get_tag_info(const T &field) {
        if constexpr (::cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return ::cpx::get_tag_info(field, "toml");
    }

    template <typename... T>
    constexpr TagInfoTuple<sizeof...(T)> get_tag_info_from_tuple(const std::tuple<T...> &fields) {
        TagInfoTuple<sizeof...(T)> ts       = {};
        bool                       is_array = sizeof...(T) > 0;

        tuple_for_each(fields, [&](const auto &field, size_t i) {
            TagInfo &t = ts.ts[i] = ::cpx::toml::get_tag_info(field);
            is_array &= t.positional;
        });

        ts.is_obj = !is_array;
        return ts;
    }


    template <typename T>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect : std::bool_constant<(Reflect<T>::value || cpx::Reflect<T>::value) && !cpx::is_time_v<T>> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, typename cpx::Reflect<T>::type>;

    template <typename T>
    using const_reflect_t =
        std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, typename cpx::Reflect<T>::const_type>;

    template <typename T>
    constexpr decltype(auto) reflect_of(const T &v) {
        if constexpr (Reflect<T>::value) {
            return Reflect<T>::of(v);
        } else {
            return cpx::Reflect<T>::of(v);
        }
    }

    template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        if constexpr (Reflect<T>::value) {
            return Reflect<T>::of(v);
        } else {
            return cpx::Reflect<T>::of(v);
        }
    }
} // namespace cpx::toml

#endif
