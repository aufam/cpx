#ifndef CPX_RESULT_H
#define CPX_RESULT_H

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <optional>

namespace cpx {
    /// A C++17 Result<T, E> type representing either a success value (Ok) or an error (Err).
    /// Provides monadic operations: and_then, transform, map_err, or_else.
    ///
    /// @code
    /// cpx::Result<int> parse_int(std::string_view s) {
    ///     try { return std::stoi(std::string(s)); }
    ///     catch (...) { return std::runtime_error("not a number"); }
    /// }
    ///
    /// auto result = parse_int("42")
    ///     .and_then([](int n) -> cpx::Result<int> { return n * 2; })
    ///     .transform([](int n) { return n + 1; });
    ///
    /// if (result.is_ok()) fmt::println("ok: {}", *result);
    ///
    /// int number = result; // implicit conversion or throw E
    ///
    /// @endcode
    template <typename T, typename E = std::runtime_error>
    class [[nodiscard]] Result {
        std::variant<T, E> data;

    public:
        using value_type = T;
        using error_type = E;

        template <typename U, typename = std::enable_if_t<std::is_constructible_v<decltype(data), U &&>>>
        constexpr Result(U &&val)
            : data(std::forward<U>(val)) {}

        // Internal construction helpers — use in_place_index for direct initialization.
        static constexpr Result ok(T &&v) {
            return Result(std::in_place_index<0>, std::move(v));
        }

        static constexpr Result ok(const T &v) {
            return Result(std::in_place_index<0>, v);
        }

        static constexpr Result err(E &&e) {
            return Result(std::in_place_index<1>, std::move(e));
        }

        static constexpr Result err(const E &e) {
            return Result(std::in_place_index<1>, e);
        }

        /// Returns true if this is a success value.
        constexpr bool is_ok() const noexcept {
            return data.index() == 0;
        }

        /// Returns true if this is an error value.
        constexpr bool is_err() const noexcept {
            return data.index() == 1;
        }

        constexpr operator T &() & {
            if (is_err())
                throw error();
            return value();
        }

        constexpr operator const T &() const & {
            if (is_err())
                throw error();
            return value();
        }

        constexpr operator T() && {
            if (is_err())
                throw std::move(error());
            return std::move(value());
        }

        /// Access the success value; throws bad_variant_access if this is an error.
        constexpr T &value() & {
            return std::get<0>(data);
        }

        constexpr const T &value() const & {
            return std::get<0>(data);
        }

        constexpr T &&value() && {
            return std::get<0>(std::move(data));
        }

        /// Access the error value; throws bad_variant_access if this is a success.
        constexpr E &error() & {
            return std::get<1>(data);
        }

        constexpr const E &error() const & {
            return std::get<1>(data);
        }

        constexpr E &&error() && {
            return std::get<1>(std::move(data));
        }

        /// Returns the success value or a default if this is an error.
        constexpr T value_or(T &&default_val) const & {
            if (is_ok())
                return value();
            return std::forward<T>(default_val);
        }

        constexpr T value_or(T &&default_val) && {
            if (is_ok())
                return std::move(value());
            return std::forward<T>(default_val);
        }

        /// Monadic and_then: applies f (T -> Result<U, E>) if this is Ok.
        template <typename F>
        constexpr auto and_then(F &&f) & -> std::invoke_result_t<F, T &> {
            using Ret = std::invoke_result_t<F, T &>;
            if (is_ok())
                return std::forward<F>(f)(value());
            return Ret::err(error());
        }

        template <typename F>
        constexpr auto and_then(F &&f) const & -> std::invoke_result_t<F, const T &> {
            using Ret = std::invoke_result_t<F, const T &>;
            if (is_ok())
                return std::forward<F>(f)(value());
            return Ret::err(error());
        }

        template <typename F>
        constexpr auto and_then(F &&f) && -> std::invoke_result_t<F, T &&> {
            using Ret = std::invoke_result_t<F, T &&>;
            if (is_ok())
                return std::forward<F>(f)(std::move(value()));
            return Ret::err(std::move(error()));
        }

        /// Monadic transform: applies f (T -> U) if this is Ok, returning Result<U, E>.
        template <typename F>
        constexpr auto transform(F &&f) const & -> Result<std::invoke_result_t<F, const T &>, E> {
            using U = std::invoke_result_t<F, const T &>;
            if (is_ok())
                return Result<U, E>::ok(std::forward<F>(f)(value()));
            return Result<U, E>::err(error());
        }

        template <typename F>
        constexpr auto transform(F &&f) && -> Result<std::invoke_result_t<F, T &&>, E> {
            using U = std::invoke_result_t<F, T &&>;
            if (is_ok())
                return Result<U, E>::ok(std::forward<F>(f)(std::move(value())));
            return Result<U, E>::err(std::move(error()));
        }

        /// map_err: applies f (E -> F) if this is Err, returning Result<T, F>.
        template <typename G>
        constexpr auto map_err(G &&g) const & -> Result<T, std::invoke_result_t<G, const E &>> {
            using F2 = std::invoke_result_t<G, const E &>;
            if (is_err())
                return Result<T, F2>::err(std::forward<G>(g)(error()));
            return Result<T, F2>::ok(value());
        }

        template <typename G>
        constexpr auto map_err(G &&g) && -> Result<T, std::invoke_result_t<G, E &&>> {
            using F2 = std::invoke_result_t<G, E &&>;
            if (is_err())
                return Result<T, F2>::err(std::forward<G>(g)(std::move(error())));
            return Result<T, F2>::ok(std::move(value()));
        }

        /// or_else: calls f (E -> Result<T, E2>) if this is an Err.
        template <typename G>
        constexpr auto or_else(G &&g) const & -> std::invoke_result_t<G, const E &> {
            using Ret = std::invoke_result_t<G, const E &>;
            if (is_err())
                return std::forward<G>(g)(error());
            return Ret::ok(value());
        }

        template <typename G>
        constexpr auto or_else(G &&g) && -> std::invoke_result_t<G, E &&> {
            using Ret = std::invoke_result_t<G, E &&>;
            if (is_err())
                return std::forward<G>(g)(std::move(error()));
            return Ret::ok(std::move(value()));
        }

    private:
        template <std::size_t I, typename U>
        constexpr Result(std::in_place_index_t<I> tag, U &&v)
            : data(tag, std::forward<U>(v)) {}
    };

    template <typename E>
    class [[nodiscard]] Result<void, E> {
        std::optional<E> data;

    public:
        using value_type = void;
        using error_type = E;

        template <typename U, typename = std::enable_if_t<std::is_constructible_v<decltype(data), U &&>>>
        constexpr Result(U &&val)
            : data(std::forward<U>(val)) {}

        constexpr Result()
            : data(std::nullopt) {}

        // Internal construction helpers — use in_place_index for direct initialization.
        static constexpr Result ok() {
            return std::nullopt;
        }

        static constexpr Result err(E &&e) {
            return std::move(e);
        }

        static constexpr Result err(const E &e) {
            return e;
        }

        /// Returns true if this is a success value.
        constexpr bool is_ok() const noexcept {
            return !data.has_value();
        }

        /// Returns true if this is an error value.
        constexpr bool is_err() const noexcept {
            return data.has_value();
        }

        /// Access the error value; throws bad_variant_access if this is a success.
        constexpr E &error() & {
            return data.value();
        }

        constexpr const E &error() const & {
            return data.value;
        }

        constexpr E &&error() && {
            return std::move(data.value());
        }

        /// Monadic and_then: applies f (T -> Result<U, E>) if this is Ok.
        template <typename F>
        constexpr auto and_then(F &&f) const & -> std::invoke_result_t<F> {
            using Ret = std::invoke_result_t<F>;
            if (is_ok())
                return std::forward<F>(f)();
            return Ret::err(error());
        }

        template <typename F>
        constexpr auto and_then(F &&f) && -> std::invoke_result_t<F> {
            using Ret = std::invoke_result_t<F>;
            if (is_ok())
                return std::forward<F>(f)();
            return Ret::err(std::move(error()));
        }

        /// Monadic transform: applies f (T -> U) if this is Ok, returning Result<U, E>.
        template <typename F>
        constexpr auto transform(F &&f) const & -> Result<std::invoke_result_t<F>, E> {
            using U = std::invoke_result_t<F>;
            if (is_ok()) {
                std::forward<F>(f)();
                return Result<U, E>::ok();
            }
            return Result<U, E>::err(error());
        }

        template <typename F>
        constexpr auto transform(F &&f) && -> Result<std::invoke_result_t<F>, E> {
            using U = std::invoke_result_t<F>;
            if (is_ok())
                return std::forward<F>(f)();
            return Result<U, E>::err(error());
        }

        /// map_err: applies f (E -> F) if this is Err, returning Result<T, F>.
        template <typename G>
        constexpr auto map_err(G &&g) const & -> Result<void, std::invoke_result_t<G, const E &>> {
            using F2 = std::invoke_result_t<G, const E &>;
            if (is_err())
                return Result<void, F2>::err(std::forward<G>(g)(error()));
            return Result<void, F2>::ok();
        }

        template <typename G>
        constexpr auto map_err(G &&g) && -> Result<void, std::invoke_result_t<G, E &&>> {
            using F2 = std::invoke_result_t<G, E &&>;
            if (is_err())
                return Result<void, F2>::err(std::forward<G>(g)(std::move(error())));
            return Result<void, F2>::ok();
        }

        /// or_else: calls f (E -> Result<T, E2>) if this is an Err.
        template <typename G>
        constexpr auto or_else(G &&g) const & -> std::invoke_result_t<G, const E &> {
            using Ret = std::invoke_result_t<G, const E &>;
            if (is_err())
                return std::forward<G>(g)(error());
            return Ret::ok();
        }

        template <typename G>
        constexpr auto or_else(G &&g) && -> std::invoke_result_t<G, E &&> {
            using Ret = std::invoke_result_t<G, E &&>;
            if (is_err())
                return std::forward<G>(g)(std::move(error()));
            return Ret::ok();
        }
    };
} // namespace cpx

#endif
