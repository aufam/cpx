#ifndef CPX_REFLECT_BUILTIN_H
#define CPX_REFLECT_BUILTIN_H

#include <cpx/reflect.h>
#include <cpx/serde/error.h>
#include <cpx/time.h>
#include <array>

#ifndef BOOST_PFR_HPP
#    if __has_include(<boost/pfr.hpp>)
#        include <boost/pfr.hpp>
#    endif
#endif

#ifndef NEARGYE_MAGIC_ENUM_HPP
#    if __has_include(<magic_enum/magic_enum.hpp>)
#        include <magic_enum/magic_enum.hpp>
#    endif
#endif

template <>
struct cpx::WeakReflect<std::tm> {
    static void to_str(const std::tm &v, std::string &str) {
        str = cpx::tm_to_string(v);
    }

    static void from_str(std::tm &v, std::string_view str) {
        v = cpx::tm_from_string(std::string(str));
    }
};

template <>
struct cpx::WeakReflect<std::timespec> {
    static void to_str(const std::timespec &v, std::string &str) {
        str = cpx::ts_to_string(v);
    }

    static void from_str(std::timespec &v, std::string_view str) {
        v = cpx::ts_from_string(std::string(str));
    }
};

namespace cpx::detail {
    template <typename T>
    struct is_std_array : std::false_type {};

    template <typename T, size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};
} // namespace cpx::detail


#ifdef BOOST_PFR_HPP
template <typename T>
struct cpx::WeakReflect<
    T, //
    std::enable_if_t<std::is_aggregate_v<T> && !cpx::is_time_v<T> && !cpx::detail::is_std_array<T>::value>
> {

    using const_type = decltype(boost::pfr::structure_tie(std::declval<const T &>()));
    using type       = decltype(boost::pfr::structure_tie(std::declval<T &>()));

    static const_type of(const T &v) {
        return boost::pfr::structure_tie(v);
    }

    static type of(T &v) {
        return boost::pfr::structure_tie(v);
    }
};

namespace cpx::test::reflect_builtin {
    struct Foo {
        int   i;
        float f;
    };

    static_assert( //
        std::is_same_v<std::tuple<int &, float &>, typename cpx::detail::reflect_traits<cpx::WeakReflect, Foo>::type>
    );

    static_assert( //
        std::is_same_v<
            std::tuple<const int &, const float &>,
            typename cpx::detail::reflect_traits<cpx::WeakReflect, Foo>::const_type
        >
    );
}; // namespace cpx::test::reflect_builtin


#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
template <typename T>
struct cpx::WeakReflect<T, std::enable_if_t<std::is_enum_v<T>>> {
    static void from_str(T &v, std::string_view str) {
        auto e = magic_enum::enum_cast<T>(str);
        if (!e.has_value()) {
            std::string what = "invalid value `" + std::string(str) + "`, expected one of {";
            for (auto &name : magic_enum::enum_names<T>()) {
                what += std::string(name) + ",";
            }
            what += "}";
            throw cpx::serde::error(std::move(what));
        }
        v = *e;
    }

    static void to_str(const T &v, std::string &str) {
        str = (std::string)magic_enum::enum_name(v);
    }
};
#endif
#endif
