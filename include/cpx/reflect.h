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

    template <typename T>
    struct Reflect : std::false_type {};

    template <typename T>
    struct has_reflect : std::bool_constant<Reflect<T>::value> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = typename Reflect<T>::type;

    template <typename T>
    using reflect_const_t = typename Reflect<T>::const_type;

    template <>
    struct Reflect<std::tm> {
        static constexpr bool value = true;

        using type       = std::string;
        using const_type = std::string;

        static const_type of(const std::tm &tm) {
            return cpx::tm_to_string(tm);
        }

        static auto of(std::tm &tm) {
            struct Hook {
                std::tm    &tm;
                std::string str = {};

                operator type &() {
                    return str;
                }

                ~Hook() {
                    tm = cpx::tm_from_string(str);
                }
            };

            Hook h{tm};
            return h;
        }
    };

    template <>
    struct Reflect<std::timespec> {
        static constexpr bool value = true;

        using type       = std::string;
        using const_type = std::string;

        static const_type of(const std::timespec &ts) {
            return cpx::ts_to_string(ts);
        }

        static auto of(std::timespec &ts) {
            struct Hook {
                std::timespec &ts;
                std::string    str = {};

                operator type &() {
                    return str;
                }

                ~Hook() {
                    ts = cpx::ts_from_string(str);
                }
            };

            Hook h{ts};
            return h;
        }
    };
} // namespace cpx
#endif
