#ifndef CPX_YAML_YAML_H
#define CPX_YAML_YAML_H

#include <cpx/reflect.h>

namespace cpx::yaml {
    CPX_EXPORT template <typename T>
    constexpr decltype(auto) get_tag_info(const T &field) {
        if constexpr (cpx::detail::is_tag_info_for_v<T>)
            return field.ti;
        else
            return cpx::get_tag_info(field, "yaml");
    }


    CPX_EXPORT template <typename T>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    CPX_EXPORT template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value || cpx::has_reflect_v<T>> {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::reflect_t<T>>;

    CPX_EXPORT template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::const_reflect_t<T>>;

    CPX_EXPORT template <typename T>
    constexpr decltype(auto) reflect_of(T &&v) {
        if constexpr (Reflect<std::decay_t<T>>::value)
            return Reflect<std::decay_t<T>>::of(std::forward<T>(v));
        else
            return cpx::reflect_of(std::forward<T>(v));
    }
} // namespace cpx::yaml

#endif
