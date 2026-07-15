#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>

namespace cpx {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T, typename Enable = void>
    struct WeakReflect;

    CPX_EXPORT template <typename T>
    struct SelfReflect;

    CPX_EXPORT template <typename T>
    struct has_reflect;

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits;
} // namespace cpx

namespace cpx::detail {
    struct invalid_reflect_type {
        constexpr invalid_reflect_type(int) {};

        constexpr bool operator==(bool v) const {
            return !v;
        }
    };
    static_assert(std::is_aggregate_v<invalid_reflect_type> == false);


    // T::__field_tags__
    template <typename T, typename = void>
    struct has_field_tags : std::false_type {};

    template <typename T>
    struct has_field_tags<T, std::void_t<decltype(T::__field_tags__)>> : std::true_type {};


    // T::__to_str__()
    template <typename T, typename = void>
    struct has_to_str : std::false_type {};

    template <typename T>
    struct has_to_str<T, std::void_t<decltype(T::__to_str__(std::declval<const T &>()))>> : std::true_type {};


    // T::__from_str__()
    template <typename T, typename = void>
    struct has_from_str : std::false_type {};

    template <typename T>
    struct has_from_str<T, std::void_t<decltype(T::__from_str__(std::declval<T &>(), std::declval<std::string_view>()))>>
        : std::true_type {};


    // T::__to_bytes__()
    template <typename T, typename = void>
    struct has_to_bytes : std::false_type {};

    template <typename T>
    struct has_to_bytes<T, std::void_t<decltype(T::__to_bytes__(std::declval<const T &>(), std::declval<std::string &>()))>>
        : std::true_type {};


    // T::__from_bytes__()
    template <typename T, typename = void>
    struct has_from_bytes : std::false_type {};

    template <typename T>
    struct has_from_bytes<T, std::void_t<decltype(T::__from_bytes__(std::declval<T &>(), std::declval<std::string_view>()))>>
        : std::true_type {};


    // R<T>::field_tags
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_field_tags : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_field_tags<R, T, std::void_t<decltype(R<T>::field_tags)>>
        : std::bool_constant<!std::is_same_v<invalid_reflect_type, std::decay_t<decltype(R<T>::field_tags)>>> {};


    // R<T>::to_str()
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_to_str : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_to_str<R, T, std::void_t<decltype(R<T>::to_str(std::declval<const T &>(), std::declval<std::string &>()))>>
        : std::bool_constant<!std::is_same_v<
              invalid_reflect_type,
              std::decay_t<decltype(R<T>::to_str(std::declval<const T &>(), std::declval<std::string &>()))>
          >> {};


    // R<T>::from_str()
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_from_str : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_from_str<
        R,
        T,
        std::void_t<decltype(R<T>::from_str(std::declval<T &>(), std::declval<std::string_view>()))>
    >
        : std::bool_constant<!std::is_same_v<
              invalid_reflect_type,
              std::decay_t<decltype(R<T>::from_str(std::declval<T &>(), std::declval<std::string_view>()))>
          >> {};


    // R<T>::to_bytes()
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_to_bytes : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_to_bytes<
        R,
        T,
        std::void_t<decltype(R<T>::to_bytes(std::declval<const T &>(), std::declval<std::string &>()))>
    >
        : std::bool_constant<!std::is_same_v<
              invalid_reflect_type,
              std::decay_t<decltype(R<T>::to_bytes(std::declval<const T &>(), std::declval<std::string &>()))>
          >> {};


    // R<T>::from_bytes()
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_from_bytes : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_from_bytes<
        R,
        T,
        std::void_t<decltype(R<T>::from_bytes(std::declval<T &>(), std::declval<std::string_view>()))>
    >
        : std::bool_constant<!std::is_same_v<
              invalid_reflect_type,
              std::decay_t<decltype(R<T>::from_bytes(std::declval<T &>(), std::declval<std::string_view>()))>
          >> {};


    // R<T>::type && R<T>::const_type && R<T>::of(T&) && R<T>::of(const T&)
    template <template <typename> typename Reflect, typename T, typename Enable = void>
    struct has_reflect_default_traits : std::false_type {};

    template <template <typename> typename Reflect, typename T>
    struct has_reflect_default_traits<
        Reflect,
        T,
        std::void_t<
            typename Reflect<T>::type,
            typename Reflect<T>::const_type,
            decltype(Reflect<T>::of(std::declval<T &>())),
            decltype(Reflect<T>::of(std::declval<const T &>()))
        >
    > : std::true_type {};


    // has_reflect_traits
    template <template <typename> typename R, typename T>
    struct has_reflect_traits
        : std::bool_constant<
              detail::has_reflect_default_traits<R, T>::value //
              || detail::has_reflect_field_tags<R, T>::value  //
              || detail::has_reflect_to_str<R, T>::value      //
              || detail::has_reflect_from_str<R, T>::value    //
              || detail::has_reflect_to_bytes<R, T>::value    //
              || detail::has_reflect_from_bytes<R, T>::value
          > {};

    // reflect_traits
    template <template <typename> typename R, typename T, typename Enable = void>
    struct reflect_traits;

    // default reflect traits
    template <template <typename> typename R, typename T>
    struct reflect_traits<R, T, std::enable_if_t<has_reflect_default_traits<R, T>::value>> {
        static constexpr bool has_default_traits = true;
        static constexpr bool has_to_str         = false;
        static constexpr bool has_from_str       = false;
        static constexpr bool has_to_bytes       = false;
        static constexpr bool has_from_bytes     = false;

        using type       = typename R<T>::type;
        using const_type = typename R<T>::const_type;

        static constexpr decltype(auto) of(T &v) {
            return R<T>::of(v);
        }

        static constexpr decltype(auto) of(const T &v) {
            return R<T>::of(v);
        }
    };

    // custom reflect traits
    template <template <typename> typename R, typename T>
    struct reflect_traits<R, T, std::enable_if_t<!has_reflect_default_traits<R, T>::value>> {
        static constexpr bool has_default_traits = detail::has_reflect_field_tags<R, T>::value;
        static constexpr bool has_to_str         = detail::has_reflect_to_str<R, T>::value;
        static constexpr bool has_from_str       = detail::has_reflect_from_str<R, T>::value;
        static constexpr bool has_to_bytes       = detail::has_reflect_to_bytes<R, T>::value;
        static constexpr bool has_from_bytes     = detail::has_reflect_from_bytes<R, T>::value;

        static constexpr decltype(auto) of(T &v) {
            if constexpr (has_default_traits) {
                return apply_field_tags(v, R<T>::field_tags);
            }
        }

        static constexpr decltype(auto) of(const T &v) {
            if constexpr (has_default_traits) {
                return apply_field_tags(v, R<T>::field_tags);
            }
        }

        using type       = decltype(of(std::declval<T &>()));
        using const_type = decltype(of(std::declval<const T &>()));

        static constexpr void to_str(const T &v, std::string &str) {
            if constexpr (has_to_str) {
                return R<T>::to_str(v, str);
            }
        }

        static constexpr void from_str(T &v, std::string_view str) {
            if constexpr (has_from_str) {
                return R<T>::from_str(v, str);
            }
        }

        static constexpr void to_bytes(const T &v, std::string &str) {
            if constexpr (has_to_bytes) {
                return R<T>::to_bytes(v, str);
            }
        }

        static constexpr void from_bytes(T &v, std::string_view str) {
            if constexpr (has_from_bytes) {
                return R<T>::from_bytes(v, str);
            }
        }
    };


    // reflect_traits_selector
    template <typename T, template <typename> typename... Rs>
    struct reflect_traits_selector;

    template <typename T>
    struct reflect_traits_selector<T> {
        static_assert(sizeof(T) == 0, "No reflect_traits specialization found.");
    };

    template <typename T, template <typename> typename R, template <typename> typename... Rs>
    struct reflect_traits_selector<T, R, Rs...>
        : std::conditional_t<
              has_reflect_traits<R, T>::value, //
              reflect_traits<R, T>,
              reflect_traits_selector<T, Rs...>
          > {};

} // namespace cpx::detail

namespace cpx {
    template <typename T>
    struct SelfReflect {
        static constexpr bool has_field_tags = cpx::detail::has_field_tags<T>::value;
        static constexpr bool has_to_str     = cpx::detail::has_to_str<T>::value;
        static constexpr bool has_from_str   = cpx::detail::has_to_str<T>::value;
        static constexpr bool has_to_bytes   = cpx::detail::has_to_bytes<T>::value;
        static constexpr bool has_from_bytes = cpx::detail::has_from_bytes<T>::value;

        static constexpr bool value = has_field_tags || (has_from_str && has_to_str) || (has_from_bytes && has_to_bytes);

        static constexpr const auto &field_tags = []() {
            if constexpr (has_field_tags) {
                return T::__field_tags__;
            } else {
                return cpx::detail::invalid_reflect_type(0);
            }
        }();

        static constexpr decltype(auto) to_str(const T &v, std::string &str) {
            if constexpr (has_to_str) {
                return T::__to_str__(v, str);
            } else {
                return cpx::detail::invalid_reflect_type(0);
            }
        }

        static constexpr decltype(auto) from_str(T &v, std::string_view str) {
            if constexpr (has_from_str) {
                return T::__from_str__(v, str);
            } else {
                return cpx::detail::invalid_reflect_type(0);
            }
        }

        static constexpr decltype(auto) to_bytes(const T &v, std::string &str) {
            if constexpr (has_to_bytes) {
                return T::__to_bytes__(v, str);
            } else {
                return cpx::detail::invalid_reflect_type(0);
            }
        }

        static constexpr decltype(auto) from_bytes(T &v, std::string_view str) {
            if constexpr (has_from_bytes) {
                return T::__from_bytes__(v, str);
            } else {
                return cpx::detail::invalid_reflect_type(0);
            }
        }
    };

    template <>
    struct SelfReflect<void> : std::false_type {};

    template <typename T>
    struct has_reflect
        : std::bool_constant<
              cpx::detail::has_reflect_traits<cpx::Reflect, T>::value ||     //
              cpx::detail::has_reflect_traits<cpx::SelfReflect, T>::value || //
              cpx::detail::has_reflect_traits<cpx::WeakReflect, T>::value
          > {};

    template <typename T>
    struct reflect_traits : cpx::detail::reflect_traits_selector<T, cpx::Reflect, cpx::SelfReflect, cpx::WeakReflect> {};
} // namespace cpx

namespace cpx::test::self_reflect {
    struct foo {
        int a, b;

        static constexpr std::tuple __field_tags__ = {
            cpx::field<&foo::a> = "a",
            cpx::field<&foo::b> = "b,skipmissing",
        };
    };

    static_assert(cpx::SelfReflect<int>::field_tags == false);
    static_assert(std::get<0>(cpx::SelfReflect<foo>::field_tags).tag.key == "a");
    static_assert(std::get<1>(cpx::SelfReflect<foo>::field_tags).tag.key == "b");
    static_assert(std::get<1>(cpx::SelfReflect<foo>::field_tags).tag.skipmissing);

    static_assert(cpx::detail::reflect_traits<cpx::SelfReflect, foo>::has_default_traits == true);
} // namespace cpx::test::self_reflect

#endif
