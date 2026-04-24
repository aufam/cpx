#ifndef CPX_OPTIONAL_H
#define CPX_OPTIONAL_H

#include <optional>
#include <type_traits>

namespace cpx {
    template <typename T>
    struct is_optional : std::false_type {};

    template <typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    template <typename T>
    constexpr bool is_optional_v = is_optional<T>::value;

    /// Monadic and_then: applies f (T -> optional<U>) if the optional has a value.
    template <typename T, typename F>
    constexpr auto and_then(std::optional<T> &&opt, F &&f)
        -> decltype(std::forward<F>(f)(std::move(*opt))) {
        if (opt.has_value())
            return std::forward<F>(f)(std::move(*opt));
        return std::nullopt;
    }

    template <typename T, typename F>
    constexpr auto and_then(const std::optional<T> &opt, F &&f)
        -> decltype(std::forward<F>(f)(*opt)) {
        if (opt.has_value())
            return std::forward<F>(f)(*opt);
        return std::nullopt;
    }

    /// Monadic transform: applies f (T -> U) if the optional has a value, returning optional<U>.
    template <typename T, typename F>
    constexpr auto transform(std::optional<T> &&opt, F &&f)
        -> std::optional<std::invoke_result_t<F, T>> {
        if (opt.has_value())
            return std::forward<F>(f)(std::move(*opt));
        return std::nullopt;
    }

    template <typename T, typename F>
    constexpr auto transform(const std::optional<T> &opt, F &&f)
        -> std::optional<std::invoke_result_t<F, const T &>> {
        if (opt.has_value())
            return std::forward<F>(f)(*opt);
        return std::nullopt;
    }

    /// Monadic or_else: calls f (-> optional<T>) if the optional is empty.
    template <typename T, typename F>
    constexpr std::optional<T> or_else(std::optional<T> &&opt, F &&f) {
        static_assert(
            std::is_invocable_r_v<std::optional<T>, F>,
            "or_else: F must return optional<T>"
        );
        if (!opt.has_value())
            return std::forward<F>(f)();
        return std::move(opt);
    }

    template <typename T, typename F>
    constexpr std::optional<T> or_else(const std::optional<T> &opt, F &&f) {
        static_assert(
            std::is_invocable_r_v<std::optional<T>, F>,
            "or_else: F must return optional<T>"
        );
        if (!opt.has_value())
            return std::forward<F>(f)();
        return opt;
    }

    /// value_or_else: returns the value if present, otherwise calls f (-> T).
    template <typename T, typename F>
    constexpr T value_or_else(std::optional<T> &&opt, F &&f) {
        static_assert(std::is_invocable_r_v<T, F>, "value_or_else: F must return T");
        if (opt.has_value())
            return std::move(*opt);
        return std::forward<F>(f)();
    }

    template <typename T, typename F>
    constexpr T value_or_else(const std::optional<T> &opt, F &&f) {
        static_assert(std::is_invocable_r_v<T, F>, "value_or_else: F must return T");
        if (opt.has_value())
            return *opt;
        return std::forward<F>(f)();
    }
} // namespace cpx

#endif
