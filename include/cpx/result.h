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
    class [[nodiscard]] Result : protected std::variant<T, E> {
    public:
        using value_type = T;
        using error_type = E;

        using std::variant<T, E>::variant;

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
            return this->index() == 0;
        }

        /// Returns true if this is an error value.
        constexpr bool is_err() const noexcept {
            return this->index() == 1;
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
            return std::get<0>(static_cast<std::variant<T, E> &>(*this));
        }

        constexpr const T &value() const & {
            return std::get<0>(static_cast<const std::variant<T, E> &>(*this));
        }

        constexpr T &&value() && {
            return std::move(std::get<0>(static_cast<std::variant<T, E> &>(*this)));
        }

        /// Access the error value; throws bad_variant_access if this is a success.
        constexpr E &error() & {
            return std::get<1>(static_cast<std::variant<T, E> &>(*this));
        }

        constexpr const E &error() const & {
            return std::get<1>(static_cast<const std::variant<T, E> &>(*this));
        }

        constexpr E &&error() && {
            return std::move(std::get<1>(static_cast<std::variant<T, E> &>(*this)));
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
            if (is_err())
                return Result<U, E>::err(error());

            if constexpr (std::is_same_v<U, void>) {
                std::forward<F>(f)(value());
                return Result<U, E>::ok();
            } else {
                return Result<U, E>::ok(std::forward<F>(f)(value()));
            }
        }

        template <typename F>
        constexpr auto transform(F &&f) && -> Result<std::invoke_result_t<F, T &&>, E> {
            using U = std::invoke_result_t<F, T &&>;
            if (is_err())
                return Result<U, E>::err(std::move(error()));

            if constexpr (std::is_same_v<U, void>) {
                std::forward<F>(f)(std::move(value()));
                return Result<U, E>::ok();
            } else {
                return Result<U, E>::ok(std::forward<F>(f)(std::move(value())));
            }
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
            : std::variant<T, E>(tag, std::forward<U>(v)) {}
    };

    template <typename E>
    class [[nodiscard]] Result<void, E> : protected std::optional<E> {
    public:
        using value_type = void;
        using error_type = E;

        using std::optional<E>::optional;

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
            return !this->has_value();
        }

        /// Returns true if this is an error value.
        constexpr bool is_err() const noexcept {
            return this->has_value();
        }

        /// Access the error value; throws bad_variant_access if this is a success.
        constexpr E &error() & {
            return this->value();
        }

        constexpr const E &error() const & {
            return this->value();
        }

        constexpr E &&error() && {
            return std::move(this->value());
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
            if (is_err())
                return Result<U, E>::err(error());

            if constexpr (std::is_same_v<U, void>) {
                std::forward<F>(f)();
                return Result<U, E>::ok();
            } else {
                return Result<U, E>::ok(std::forward<F>(f)());
            }
        }

        template <typename F>
        constexpr auto transform(F &&f) && -> Result<std::invoke_result_t<F>, E> {
            using U = std::invoke_result_t<F>;
            if (is_err())
                return Result<U, E>::err(std::move(error()));

            if constexpr (std::is_same_v<U, void>) {
                std::forward<F>(f)();
                return Result<U, E>::ok();
            } else {
                return Result<U, E>::ok(std::forward<F>(f)());
            }
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
