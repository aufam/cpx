#ifndef CPX_FMT_H
#define CPX_FMT_H

#include <cpx/serde/serialize.h>
#include <cpx/tag_info.h>
#include <cpx/reflect.h>
#include <optional>
#include <variant>

#ifndef FMT_RANGES_H_
#    include <fmt/ranges.h>
#endif

#ifndef FMT_CHRONO_H_
#    include <fmt/chrono.h>
#endif

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

template <typename T, typename TI>
struct fmt::formatter<cpx::TagInfoFor<T, TI>, char, std::enable_if_t<fmt::is_formattable<std::decay_t<T>, char>::value>>
    : fmt::formatter<std::decay_t<T>> {
    fmt::context::iterator format(const cpx::TagInfoFor<T, TI> &v, fmt::context &c) const {
        if (auto &ti = v.ti; ti.key != "") {
            fmt::context::iterator out = c.out();
            out                        = fmt::format_to(out, "{}=", ti.key);
        }
        return fmt::formatter<std::decay_t<T>>::format(v.value, c);
    }
};

template <typename T>
struct fmt::formatter<cpx::Tag<T>, char, std::enable_if_t<fmt::is_formattable<std::decay_t<T>, char>::value>>
    : fmt::formatter<std::decay_t<T>> {
    fmt::context::iterator format(const cpx::Tag<T> &v, fmt::context &c) const {
        if (cpx::TagInfo ti = cpx::get_tag_info(v, "fmt"); ti.key != "") {
            fmt::context::iterator out = c.out();
            out                        = fmt::format_to(out, "{}=", ti.key);
        }
        return fmt::formatter<std::decay_t<T>>::format(v.get_value(), c);
    }
};

template <typename T>
struct fmt::formatter<std::optional<T>, char, std::enable_if_t<fmt::is_formattable<T, char>::value>> : fmt::formatter<T> {
    fmt::context::iterator format(const std::optional<T> &v, fmt::context &c) const {
        if (v.has_value())
            return fmt::formatter<T>::format(*v, c);
        fmt::context::iterator out = c.out();
        return fmt::format_to(out, "null");
    }
};

template <typename... T>
struct fmt::formatter<std::variant<T...>, char, std::enable_if_t<(fmt::is_formattable<T, char>::value && ...)>> {
    constexpr auto parse(fmt::format_parse_context &ctx) {
        return ctx.begin();
    }

    fmt::context::iterator format(const std::variant<T...> &v, fmt::context &c) const {
        fmt::context::iterator out = c.out();
        return std::visit(
            [&](const auto &var) {
                if constexpr (std::is_same_v<std::decay_t<decltype(var)>, std::string> ||
                              std::is_same_v<std::decay_t<decltype(var)>, std::string_view>)
                    return fmt::format_to(out, "{:?}", var);
                else
                    return fmt::format_to(out, "{}", var);
            },
            v
        );
    }
};

template <>
struct fmt::formatter<std::timespec> : fmt::formatter<std::tm> {
    fmt::context::iterator format(const std::timespec &ts, fmt::context &c) const {
        std::tm tm;
#if defined(_WIN32)
        _gmtime64_s(&tm, &ts.tv_sec);
#else
        ::gmtime_r(&ts.tv_sec, &tm);
#endif

        auto out = fmt::formatter<std::tm>::format(tm, c);
        out      = fmt::format_to(out, ".{:03}", ts.tv_nsec / 1000000);
        return out;
    }
};

template <typename T>
struct fmt::formatter<T, char, std::enable_if_t<cpx::has_reflect_v<T> && !cpx::is_time_v<T>>>
    : fmt::formatter<typename cpx::Reflect<T>::const_type> {
    fmt::context::iterator format(const T &v, fmt::context &c) const {
        return fmt::formatter<typename cpx::Reflect<T>::const_type>::format(cpx::Reflect<T>::of(v), c);
    }
};

#ifdef BOOST_PFR_HPP
template <typename S>
struct fmt::formatter<S, char, std::enable_if_t<std::is_aggregate_v<S> && !cpx::has_reflect_v<S> && !cpx::is_time_v<S>>> {
    constexpr auto parse(fmt::format_parse_context &ctx) {
        return ctx.begin();
    }

    fmt::context::iterator format(const S &v, fmt::context &c) const {
        fmt::context::iterator out = c.out();

        auto tpl =
            std::apply([&](auto &...item) { return cpx::tie_if<fmt::is_formattable>(item...); }, boost::pfr::structure_tie(v));

        return fmt::format_to(out, "{}", tpl);
    }
};
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
template <typename S>
struct fmt::formatter<S, char, std::enable_if_t<std::is_enum_v<S> && !cpx::has_reflect_v<S>>> : fmt::formatter<std::string_view> {
    fmt::context::iterator format(S v, fmt::context &c) const {
        fmt::context::iterator out = c.out();
        return fmt::format_to(out, "{}", magic_enum::enum_name(v));
    }
};
#endif

#endif
