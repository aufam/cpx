#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>
#include <cpx/time.h>

namespace cpx {
    template <typename T>
    struct member_pointer_traits;

    template <typename Class, typename Member>
    struct member_pointer_traits<Member Class::*> {
        using class_type  = Class;
        using member_type = Member;
    };

    template <auto MemberPtr>
    struct Field {
        using traits = member_pointer_traits<decltype(MemberPtr)>;

        using class_type  = typename traits::class_type;
        using member_type = typename traits::member_type;

        using const_ref = cpx::TagInfoFor<const member_type &, const TagInfo &>;
        using ref       = cpx::TagInfoFor<member_type &, const TagInfo &>;
    };

    template <auto... MemberPtrs>
    struct Fields;

    template <>
    struct Fields<> {
        static constexpr bool value = true;

        using type       = std::tuple<>;
        using const_type = std::tuple<>;
    };

    template <auto MemberPtr, auto... MemberPtrs>
    struct Fields<MemberPtr, MemberPtrs...> {
        static constexpr bool value = true;

        using const_type = std::tuple<typename Field<MemberPtr>::const_ref, typename Field<MemberPtrs>::const_ref...>;
        using type       = std::tuple<typename Field<MemberPtr>::ref, typename Field<MemberPtrs>::ref...>;
    };

    template <typename T, auto... MemberPtrs>
    struct Reflect {
        static constexpr bool value = sizeof...(MemberPtrs) > 0;

        using type = std::tuple<
            std::conditional_t<std::is_const_v<T>, typename Field<MemberPtrs>::const_ref, typename Field<MemberPtrs>::ref>...>;

        constexpr Reflect(T &p)
            : p(p) {}

    protected:
        T &p;
    };

    template <>
    struct Reflect<void> : std::false_type {};

    template <>
    struct Reflect<const void> : std::false_type {};

    template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = typename Reflect<T>::type;
} // namespace cpx

template <>
struct cpx::Reflect<const std::tm> {
    static constexpr bool value = true;

    using type = std::string;

    const std::tm &p;

    constexpr Reflect(const std::tm &p)
        : p(p) {}

    operator type() const {
        return cpx::tm_to_string(p);
    }
};

template <>
struct cpx::Reflect<std::tm> {
    static constexpr bool value = true;

    using type = std::string;

    std::tm    &p;
    std::string str;

    Reflect(std::tm &p)
        : p(p) {}

    operator type &() {
        return str;
    }

    ~Reflect() {
        p = cpx::tm_from_string(str);
    }
};

#endif
