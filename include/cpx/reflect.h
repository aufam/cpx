#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>

namespace cpx {
    CPX_EXPORT template <typename T, typename Enable = void>
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
    CPX_EXPORT template <typename Derived, auto... MemberPtr>
    struct Fields;

    CPX_EXPORT template <typename Derived>
    struct Fields<Derived> {
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

    CPX_EXPORT template <typename Derived, auto MemberPtr, auto... MemberPtrs>
    struct Fields<Derived, MemberPtr, MemberPtrs...> {
        static constexpr bool value = true;

        using class_type = typename cpx::detail::member_pointer_traits<decltype(MemberPtr)>::class_type;

        using const_type = //
            std::tuple<
                TagInfoFor<
                    const typename cpx::detail::member_pointer_traits<decltype(MemberPtr)>::member_type &,
                    const TagInfo &>,
                TagInfoFor<
                    const typename cpx::detail::member_pointer_traits<decltype(MemberPtrs)>::member_type &,
                    const TagInfo &>...>;

        using type = //
            std::tuple<
                TagInfoFor<typename cpx::detail::member_pointer_traits<decltype(MemberPtr)>::member_type &, const TagInfo &>,
                TagInfoFor<typename cpx::detail::member_pointer_traits<decltype(MemberPtrs)>::member_type &, const TagInfo &>...>;

        using tags_type = std::tuple<const TagInfo &, std::conditional_t<false, decltype(MemberPtrs), const TagInfo &>...>;

        static const_type of(const class_type &obj) {
            return impl(obj, std::index_sequence_for<decltype(MemberPtrs)...>{});
        }

        static type of(class_type &obj) {
            return impl(obj, std::index_sequence_for<decltype(MemberPtrs)...>{});
        }

    private:
        template <std::size_t... Is>
        static const_type impl(const class_type &obj, std::index_sequence<Is...>) {
            tags_type tags = Derived::tags();
            return std::make_tuple(
                cpx::tag_tie(obj.*MemberPtr, std::get<0>(tags)), cpx::tag_tie(obj.*MemberPtrs, std::get<Is + 1>(tags))...
            );
        }

        template <std::size_t... Is>
        static type impl(class_type &obj, std::index_sequence<Is...>) {
            tags_type tags = Derived::tags();
            return std::make_tuple(
                cpx::tag_tie(obj.*MemberPtr, std::get<0>(tags)), cpx::tag_tie(obj.*MemberPtrs, std::get<Is + 1>(tags))...
            );
        }
    };
} // namespace cpx

namespace cpx::weak {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    CPX_EXPORT template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value> {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    using reflect_t = typename Reflect<T>::type;

    CPX_EXPORT template <typename T>
    using const_reflect_t = typename Reflect<T>::const_type;

    CPX_EXPORT template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        return Reflect<std::remove_const_t<T>>::of(v);
    }
} // namespace cpx::weak

namespace cpx {
    CPX_EXPORT template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value || cpx::weak::has_reflect_v<T>> {};

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::weak::reflect_t<T>>;

    CPX_EXPORT template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::weak::const_reflect_t<T>>;

    CPX_EXPORT template <typename T>
    constexpr decltype(auto) reflect_of(T &&v) {
        if constexpr (Reflect<std::decay_t<T>>::value)
            return Reflect<std::decay_t<T>>::of(std::forward<T>(v));
        else
            return cpx::weak::reflect_of(std::forward<T>(v));
    }
} // namespace cpx

#endif
