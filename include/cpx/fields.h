#ifndef CPX_FIELDS_H
#define CPX_FIELDS_H

#include <cpx/tag_info.h>

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

#endif
