#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>

namespace cpx {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect;

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits;
} // namespace cpx

namespace cpx::weak {
    CPX_EXPORT template <typename T, typename Enable = void>
    struct Reflect;

    CPX_EXPORT template <typename T>
    struct has_reflect;

    CPX_EXPORT template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    CPX_EXPORT template <typename T>
    struct reflect_traits;
} // namespace cpx::weak

namespace cpx::detail {
    // T::__field_tags__
    template <typename T, typename = void>
    struct has_field_tags : std::false_type {};

    template <typename T>
    struct has_field_tags<T, std::void_t<decltype(T::__field_tags__)>> : std::true_type {};


    // R<T>::field_tags
    template <template <typename> typename R, typename T, typename Enable = void>
    struct has_reflect_field_tags : std::false_type {};

    template <template <typename> typename R, typename T>
    struct has_reflect_field_tags<R, T, std::void_t<decltype(R<T>::field_tags)>> : std::true_type {};


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

    template <template <typename> typename R, typename T>
    struct has_reflect_traits
        : std::bool_constant<
              detail::has_reflect_default_traits<R, T>::value || //
              detail::has_reflect_field_tags<R, T>::value ||     //
              detail::has_field_tags<T>::value
          > {};


    template <template <typename> typename R, typename T, typename Enable = void>
    struct reflect_traits;

    // R<T>::type && R<T>::const_type && R<T>::of(T&) && R<T>::of(const T&)
    template <template <typename> typename R, typename T>
    struct reflect_traits<R, T, std::enable_if_t<has_reflect_default_traits<R, T>::value>> {
        using type       = typename R<T>::type;
        using const_type = typename R<T>::const_type;

        static constexpr decltype(auto) of(T &v) {
            return R<T>::of(v);
        }

        static constexpr decltype(auto) of(const T &v) {
            return R<T>::of(v);
        }
    };

    // R<T>::field_tags
    template <template <typename> typename R, typename T>
    struct reflect_traits<
        R,
        T,
        std::enable_if_t<!has_reflect_default_traits<R, T>::value && has_reflect_field_tags<R, T>::value>
    > {
        using type       = decltype(apply_field_tags(std::declval<T &>(), R<T>::field_tags));
        using const_type = decltype(apply_field_tags(std::declval<const T &>(), R<T>::field_tags));

        static constexpr decltype(auto) of(T &v) {
            return apply_field_tags(v, R<T>::field_tags);
        }

        static constexpr decltype(auto) of(const T &v) {
            return apply_field_tags(v, R<T>::field_tags);
        }
    };

    // T::__field_tags__
    template <template <typename> typename R, typename T>
    struct reflect_traits<
        R,
        T,
        std::enable_if_t<
            !has_reflect_default_traits<R, T>::value && //
            !has_reflect_field_tags<R, T>::value &&     //
            has_field_tags<T>::value
        >
    > {
        using type       = decltype(apply_field_tags(std::declval<T &>(), T::__field_tags__));
        using const_type = decltype(apply_field_tags(std::declval<const T &>(), T::__field_tags__));

        static constexpr decltype(auto) of(T &v) {
            return apply_field_tags(v, T::__field_tags__);
        }

        static constexpr decltype(auto) of(const T &v) {
            return apply_field_tags(v, T::__field_tags__);
        }
    };


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

namespace cpx::weak {
    template <typename T>
    struct has_reflect : std::bool_constant<cpx::detail::has_reflect_traits<Reflect, T>::value> {};

    template <typename T>
    struct reflect_traits : cpx::detail::reflect_traits<Reflect, T> {};
} // namespace cpx::weak

namespace cpx {
    template <typename T>
    struct has_reflect
        : std::bool_constant<
              cpx::detail::has_reflect_traits<cpx::Reflect, T>::value || //
              cpx::detail::has_reflect_traits<cpx::weak::Reflect, T>::value
          > {};

    template <typename T>
    struct reflect_traits : cpx::detail::reflect_traits_selector<T, cpx::Reflect, cpx::weak::Reflect> {};
} // namespace cpx

#endif
