#ifndef CPX_OVERLOAD_H
#define CPX_OVERLOAD_H

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
    template <typename... Fs>
    struct overload : Fs... {
        using Fs::operator()...;
    };

    template <typename... Fs>
    overload(Fs...) -> overload<Fs...>;

    /// Convenience wrapper around std::visit using cpx::overload.
    template <typename Variant, typename... Fs>
    constexpr decltype(auto) visit(Variant &&v, Fs &&...fs) {
        return std::visit(overload{std::forward<Fs>(fs)...}, std::forward<Variant>(v));
    }
} // namespace cpx

#endif
