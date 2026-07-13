#ifndef CPX_FMT_H
#define CPX_FMT_H

#include <cpx/fmt_reflect.h>
#include <cpx/reflect_builtin.h>
#include <cpx/extend.h>
#include <optional>
#include <variant>

#ifndef FMT_RANGES_H_
#    include <fmt/ranges.h>
#endif

#ifndef FMT_CHRONO_H_
#    include <fmt/chrono.h>
#endif

namespace cpx::fmt::detail {
    template <typename T, typename Enable = void>
    struct is_tuple_reflect : std::false_type {};

    template <typename T>
    struct is_tuple_reflect<T, std::enable_if_t<cpx::fmt::has_reflect_v<T>>>
        : std::bool_constant<                                                      //
              cpx::is_tuple_v<typename cpx::fmt::reflect_traits<T>::const_type> && //
              ::fmt::is_formattable<typename cpx::fmt::reflect_traits<T>::const_type>::value
          > {};

    template <typename T, typename Enable = void>
    struct is_non_tuple_reflect : std::false_type {};

    template <typename T>
    struct is_non_tuple_reflect<T, std::enable_if_t<cpx::fmt::has_reflect_v<T>>>
        : std::bool_constant<
              !cpx::is_tuple_v<typename cpx::fmt::reflect_traits<T>::const_type> &&
              ::fmt::is_formattable<typename cpx::fmt::reflect_traits<T>::const_type>::value
          > {};
} // namespace cpx::fmt::detail

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
struct fmt::formatter<cpx::Extend<T>, char, std::enable_if_t<fmt::is_formattable<T>::value>> : fmt::formatter<T> {
    fmt::context::iterator format(const cpx::Extend<T> &v, fmt::context &c) const {
        return fmt::formatter<T>::format(v.get_value(), c);
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
                if constexpr (
                    std::is_same_v<std::decay_t<decltype(var)>, std::string> ||
                    std::is_same_v<std::decay_t<decltype(var)>, std::string_view>
                )
                    return fmt::format_to(out, "{:?}", var);
                else
                    return fmt::format_to(out, "{}", var);
            },
            v
        );
    }
};

template <>
struct fmt::formatter<std::timespec>
    : fmt::formatter<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>> {

    fmt::context::iterator format(const std::timespec &ts, fmt::context &ctx) const {
        constexpr auto ten_years = 24l * 3600 * 365;

        if (ts.tv_sec <= ten_years && ts.tv_sec >= 0) {
            auto out = ctx.out();
            return fmt::format_to(out, "{}", cpx::ts_to_string(ts));
        }

        auto tp = std::chrono::system_clock::time_point{std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec)
        )};

        auto tp_ms = std::chrono::floor<std::chrono::milliseconds>(tp);

        return fmt::formatter<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>>::format(tp_ms, ctx);
    }
};

template <typename T>
struct fmt::formatter<T, char, std::enable_if_t<cpx::fmt::detail::is_non_tuple_reflect<T>::value>>
    : fmt::formatter<typename cpx::fmt::reflect_traits<T>::const_type> {
    fmt::context::iterator format(const T &v, fmt::context &c) const {
        return fmt::formatter<typename cpx::fmt::reflect_traits<T>::const_type>::format(cpx::fmt::reflect_traits<T>::of(v), c);
    }
};

template <typename T>
struct fmt::formatter<
    T,
    char,
    std::enable_if_t<
        cpx::fmt::detail::is_tuple_reflect<T>::value &&         //
        !std::is_same_v<std::integral_constant<bool, false>, T> // TODO: why is this required
    >
> {

    constexpr auto parse(fmt::format_parse_context &ctx) {
        return ctx.begin();
    }

    fmt::context::iterator format(const T &v, fmt::context &c) const {
        decltype(auto) tpl       = cpx::fmt::reflect_traits<T>::of(v);
        const auto     flattened = cpx::flatten(tpl);

        const auto formattable_tpl = std::apply([](auto &...tpl) { return cpx::tie_if<fmt::is_formattable>(tpl...); }, flattened);
        fmt::context::iterator out = c.out();
        return fmt::format_to(out, "{}", formattable_tpl);
    }
};
#endif
