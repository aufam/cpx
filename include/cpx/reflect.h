#ifndef CPX_REFLECT_H
#define CPX_REFLECT_H

#include <cpx/tag_info.h>
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

namespace cpx {
    template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
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

template <>
struct cpx::weak::Reflect<std::tm> {
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

            ~Hook() noexcept(false) {
                if (std::uncaught_exceptions() > 0)
                    return;
                tm = cpx::tm_from_string(str);
            }
        };

        Hook h{tm};
        return h;
    }
};

template <>
struct cpx::weak::Reflect<std::timespec> {
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

            ~Hook() noexcept(false) {
                if (std::uncaught_exceptions() > 0)
                    return;
                ts = cpx::ts_from_string(str);
            }
        };

        Hook h{ts};
        return h;
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
struct cpx::weak::
    Reflect<T, std::enable_if_t<std::is_aggregate_v<T> && !cpx::is_time_v<T> && !cpx::detail::is_std_array<T>::value>> {
    static constexpr bool value = true;

    using const_type = decltype(boost::pfr::structure_tie(std::declval<const T &>()));
    using type       = decltype(boost::pfr::structure_tie(std::declval<T &>()));

    static const_type of(const T &v) {
        return boost::pfr::structure_tie(v);
    }

    static type of(T &v) {
        return boost::pfr::structure_tie(v);
    }
};
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
template <typename T>
struct cpx::weak::Reflect<T, std::enable_if_t<std::is_enum_v<T>>> {
    static constexpr bool value = true;

    using const_type = std::string_view;
    using type       = std::string;

    static const_type of(const T &v) {
        return magic_enum::enum_name(v);
    }

    struct Hook {
        T   &v;
        type str = {};

        operator type &() {
            return str;
        }

        ~Hook() noexcept(false) {
            if (std::uncaught_exceptions() > 0)
                return;

            auto e = magic_enum::enum_cast<T>(str);
            if (!e.has_value()) {
                std::string what = "invalid value `" + str + "`, expected one of {";
                for (auto &name : magic_enum::enum_names<T>()) {
                    what += std::string(name) + ",";
                }
                what += "}";
                throw cpx::serde::error(std::move(what));
            }
            v = *e;
        }
    };

    static Hook of(T &v) {
        return {v};
    }
};
#endif
#endif
