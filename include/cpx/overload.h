#ifndef CPX_OVERLOAD_H
#define CPX_OVERLOAD_H

#include <cpx/nomodule.h>
#include <variant>

namespace cpx {
    /// Combines multiple callable objects into one overload set.
    /// Useful with std::visit on std::variant.
    ///
    /// @code
    /// std::variant<int, float, std::string> v = 42;
    /// std::visit(cpx::overload{
    ///     [](int i)         { fmt::println("int: {}", i); },
    ///     [](float f)       { fmt::println("float: {}", f); },
    ///     [](std::string s) { fmt::println("string: {}", s); },
    /// }, v);
    /// @endcode
    CPX_EXPORT template <typename... Fs>
    struct overload : Fs... {
        using Fs::operator()...;
    };

    CPX_EXPORT template <typename... Fs>
    overload(Fs...) -> overload<Fs...>;

    /// Convenience wrapper around std::visit using cpx::overload.
    CPX_EXPORT template <typename Variant, typename... Fs>
    constexpr decltype(auto) visit(Variant &&v, Fs &&...fs) {
        return std::visit(overload{std::forward<Fs>(fs)...}, std::forward<Variant>(v));
    }

#if __cplusplus >= 202002L
    CPX_EXPORT template <typename R, typename Variant, typename... Fs>
    auto async_visit(Variant v, Fs... fs) -> R {
        co_return co_await std::visit(overload{std::move(fs)...}, std::move(v));
    }
#endif
} // namespace cpx

#endif
