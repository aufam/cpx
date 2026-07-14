#ifndef CPX_FMT_REFLECT_H
#define CPX_FMT_REFLECT_H

#include <cpx/reflect.h>
#include <cpx/time.h>

namespace cpx::fmt {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect
        : std::bool_constant<
              ( // from reflect traits
                  cpx::detail::has_reflect_traits<cpx::fmt::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::SelfReflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::WeakReflect, T>::value
              ) &&
              !( // primitive types
                  cpx::is_time_v<T>
              )
          > {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits
        : cpx::detail::reflect_traits_selector<T, cpx::fmt::Reflect, cpx::Reflect, cpx::SelfReflect, cpx::WeakReflect> {};
} // namespace cpx::fmt

#endif
