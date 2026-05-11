#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>

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

        using const_ref = std::tuple<const member_type &, const TagInfo &>;
        using ref       = std::tuple<member_type &, const TagInfo &>;
    };

    template <typename T, auto... MemberPtrs>
    struct Reflect {
        static constexpr bool value = true;

        using type = std::tuple<
            std::conditional_t<std::is_const_v<T>, typename Field<MemberPtrs>::const_ref, typename Field<MemberPtrs>::ref>...>;

        Reflect(T &p)
            : p(p) {}

        virtual operator type() const = 0;

    protected:
        T &p;
    };
} // namespace cpx

#endif
