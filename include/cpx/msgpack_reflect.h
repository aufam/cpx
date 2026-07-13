#ifndef CPX_MSGPACK_REFLECT_H
#define CPX_MSGPACK_REFLECT_H

#include <cpx/reflect.h>
#include <cpx/time.h>

namespace cpx::msgpack {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect
        : std::bool_constant<
              ( // from reflect traits
                  cpx::detail::has_reflect_traits<cpx::msgpack::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::Reflect, T>::value ||
                  cpx::detail::has_reflect_traits<cpx::weak::Reflect, T>::value
              ) &&
              !( // primitive types
                  cpx::is_time_v<T> || std::is_enum_v<T>
              )
          > {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits : cpx::detail::reflect_traits_selector<T, cpx::msgpack::Reflect, cpx::Reflect, cpx::weak::Reflect> {};
} // namespace cpx::msgpack

#endif
