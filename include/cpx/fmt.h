#ifndef CPX_FMT_H
#define CPX_FMT_H

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

namespace cpx::fmt {
    template <typename T, typename Enable = void>
    struct Reflect : std::false_type {
        using const_type = type;
    };

    template <typename T>
    struct has_reflect : std::bool_constant<(Reflect<T>::value || cpx::has_reflect_v<T>) && !cpx::is_time_v<T>> {};

    template <typename T>
    inline constexpr bool has_reflect_v = has_reflect<T>::value;

    template <typename T>
    using reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::type, cpx::reflect_t<T>>;

    template <typename T>
    using const_reflect_t = std::conditional_t<Reflect<T>::value, typename Reflect<T>::const_type, cpx::const_reflect_t<T>>;

    template <typename T>
    constexpr decltype(auto) reflect_of(T &v) {
        if constexpr (Reflect<std::remove_const_t<T>>::value)
            return Reflect<std::remove_const_t<T>>::of(v);
        else
            return cpx::reflect_of(v);
    }
} // namespace cpx::fmt

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
struct fmt::formatter<
    T,
    char,
    std::enable_if_t<
        cpx::fmt::has_reflect_v<T> && !cpx::is_tuple_v<cpx::fmt::const_reflect_t<T>> &&
        fmt::is_formattable<cpx::fmt::const_reflect_t<T>>::value>> : fmt::formatter<cpx::fmt::const_reflect_t<T>> {
    fmt::context::iterator format(const T &v, fmt::context &c) const {
        return fmt::formatter<cpx::fmt::const_reflect_t<T>>::format(cpx::fmt::reflect_of(v), c);
    }
};

template <typename T>
struct fmt::formatter<
    T,
    char,
    std::enable_if_t<
        cpx::fmt::has_reflect_v<T> && cpx::is_tuple_v<cpx::fmt::const_reflect_t<T>> &&
        !std::is_same_v<std::integral_constant<bool, false>, T> // TODO: why is this required
        >> {

    constexpr auto parse(fmt::format_parse_context &ctx) {
        return ctx.begin();
    }

    fmt::context::iterator format(const T &v, fmt::context &c) const {
        decltype(auto) tpl       = cpx::fmt::reflect_of(v);
        const auto     flattened = cpx::flatten(tpl);

        const auto formattable_tpl = std::apply([](auto &...tpl) { return cpx::tie_if<fmt::is_formattable>(tpl...); }, tpl);
        fmt::context::iterator out = c.out();
        return fmt::format_to(out, "{}", formattable_tpl);
    }
};
#endif
