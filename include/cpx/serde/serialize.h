#ifndef CPX_SERDE_SERIALIZE_H
#define CPX_SERDE_SERIALIZE_H

#include "cpx/nomodule.h"
#include <type_traits>

namespace cpx::serde {
    CPX_EXPORT template <typename Serializer, typename From, typename Enable = void>
    struct Serialize;

    CPX_EXPORT template <typename Serializer, typename To, typename Enable = void>
    struct Dump;

    CPX_EXPORT template <typename Serializer, typename From, typename = void>
    struct is_serializable : std::false_type {};

    template <typename Serializer, typename From>
    struct is_serializable<
        Serializer,
        From,
        std::void_t<decltype(std::declval<Serialize<Serializer, From>>().from(std::declval<const From &>()))>> : std::true_type {
    };

    CPX_EXPORT template <typename Serializer, typename From>
    inline constexpr bool is_serializable_v = is_serializable<Serializer, From>::value;
} // namespace cpx::serde

#endif
