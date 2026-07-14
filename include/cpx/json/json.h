#ifndef CPX_JSON_JSON_H
#define CPX_JSON_JSON_H

#include <cpx/reflect.h>

namespace cpx::json {
    CPX_EXPORT template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "json");
    }

    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect
        : std::bool_constant<
              cpx::detail::has_reflect_traits<cpx::json::Reflect, T>::value ||
              cpx::detail::has_reflect_traits<cpx::Reflect, T>::value ||
              cpx::detail::has_reflect_traits<cpx::SelfReflect, T>::value ||
              cpx::detail::has_reflect_traits<cpx::WeakReflect, T>::value
          > {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits
        : cpx::detail::reflect_traits_selector<T, cpx::json::Reflect, cpx::Reflect, cpx::SelfReflect, cpx::WeakReflect> {};
} // namespace cpx::json

#endif
