#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>

namespace cpx {
    template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };
} // namespace cpx

namespace cpx::detail {
    template <typename T>
    struct member_pointer_traits;

    template <typename Class, typename Member>
    struct member_pointer_traits<Member Class::*> {
        using class_type  = Class;
        using member_type = Member;
    };
} // namespace cpx::detail

namespace cpx {
    template <auto MemberPtr, const TagInfo &TagRef>
    struct Field {
        static constexpr auto  member_ptr = MemberPtr;
        static constexpr auto &tag        = TagRef;

        using traits = cpx::detail::member_pointer_traits<decltype(MemberPtr)>;

        using class_type  = typename traits::class_type;
        using member_type = typename traits::member_type;

        using const_ref = TagInfoFor<const member_type &, const TagInfo &>;
        using ref       = TagInfoFor<member_type &, const TagInfo &>;
    };

    template <typename... TaggedFields>
    struct Fields;

    template <>
    struct Fields<> {
        static constexpr bool value = true;

        using const_type = std::tuple<>;
        using type       = std::tuple<>;

        template <typename T>
        static const_type of(const T &) {
            return {};
        }

        template <typename T>
        static type of(T &) {
            return {};
        }
    };

    template <typename TaggedField, typename... TaggedFields>
    struct Fields<TaggedField, TaggedFields...> {
        static constexpr bool value = true;

        using class_type = typename TaggedField::class_type;
        using const_type = std::tuple<typename TaggedField::const_ref, typename TaggedFields::const_ref...>;
        using type       = std::tuple<typename TaggedField::ref, typename TaggedFields::ref...>;

        static const_type of(const class_type &obj) {
            return std::make_tuple(
                cpx::tag_tie(obj.*TaggedField::member_ptr, TaggedField::tag),
                cpx::tag_tie(obj.*TaggedFields::member_ptr, TaggedFields::tag)...
            );
        }

        static type of(class_type &obj) {
            return std::make_tuple(
                cpx::tag_tie(obj.*TaggedField::member_ptr, TaggedField::tag),
                cpx::tag_tie(obj.*TaggedFields::member_ptr, TaggedFields::tag)...
            );
        }
    };
} // namespace cpx

namespace cpx::weak {
    template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = typename Reflect<T>::type;

    template <typename T>
    using const_reflect_t = typename Reflect<T>::const_type;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        return Reflect<std::remove_const_t<T>>::of(v);
    }
} // namespace cpx::weak

namespace cpx {
    template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value || cpx::weak::has_reflect_v<T>> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::weak::reflect_t<T>>;

    template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::weak::const_reflect_t<T>>;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &&v) {
        if constexpr (Reflect<std::decay_t<T>>::value)
            return Reflect<std::decay_t<T>>::of(std::forward<T>(v));
        else
            return cpx::weak::reflect_of(std::forward<T>(v));
    }
} // namespace cpx

#endif
