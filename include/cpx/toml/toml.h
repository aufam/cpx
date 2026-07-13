#ifndef CPX_TOML_TOML_H
#define CPX_TOML_TOML_H

#include <cpx/reflect.h>
#include <cpx/time.h>

namespace cpx::toml {
    CPX_EXPORT template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "toml");
    }

    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect
        : std::bool_constant<
              ( //
                  cpx::detail::has_reflect_traits<cpx::toml::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::weak::Reflect, T>::value
              ) &&
              !cpx::is_time_v<T> // time types are primitive in toml
          > {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits : cpx::detail::reflect_traits_selector<T, cpx::toml::Reflect, cpx::Reflect, cpx::weak::Reflect> {};
} // namespace cpx::toml

#endif
