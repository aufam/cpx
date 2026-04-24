#ifndef CPX_RESULT_H
#define CPX_RESULT_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace cpx {
    struct bad_result_access : std::runtime_error {
        explicit bad_result_access(const char *msg)
            : std::runtime_error(msg) {}
    };

    /// A C++17 Result<T, E> type representing either a success value (Ok) or an error (Err).
    /// Provides monadic operations: and_then, transform, map_err, or_else.
    ///
    /// @code
    /// cpx::Result<int> parse_int(std::string_view s) {
    ///     try { return {std::stoi(std::string(s))}; }
    ///     catch (...) { return cpx::Err{"not a number"}; }
    /// }
    ///
    /// auto result = parse_int("42")
    ///     .and_then([](int n) -> cpx::Result<int> { return {n * 2}; })
    ///     .transform([](int n) { return n + 1; });
    ///
    /// if (result) fmt::println("ok: {}", *result);
    /// @endcode
    template <typename T, typename E = std::string>
    class Result {
        static_assert(!std::is_reference_v<T>, "T must not be a reference type");
        static_assert(!std::is_reference_v<E>, "E must not be a reference type");

        std::variant<T, E> data;

    public:
        using value_type = T;
        using error_type = E;

        /// Construct from a success value (implicit conversion).
        template <
            typename U = T,
            typename   = std::enable_if_t<
                std::is_constructible_v<T, U &&> &&
                !std::is_same_v<std::decay_t<U>, Result>>>
        constexpr Result(U &&value)
            : data(std::in_place_index<0>, std::forward<U>(value)) {}

        /// Construct from an error (use Err tag to disambiguate when T and E overlap).
        template <
            typename U = E,
            typename   = std::enable_if_t<std::is_constructible_v<E, U &&>>,
            typename   = void> // extra void to allow overload
        constexpr Result(U &&err, bool /*is_err*/)
            : data(std::in_place_index<1>, std::forward<U>(err)) {}

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

        constexpr explicit operator bool() const noexcept {
            return is_ok();
        }

        /// Access the success value; throws bad_result_access if this is an error.
        constexpr T &value() & {
            if (is_err())
                throw bad_result_access("called value() on an Err Result");
            return std::get<0>(data);
        }

        constexpr const T &value() const & {
            if (is_err())
                throw bad_result_access("called value() on an Err Result");
            return std::get<0>(data);
        }

        constexpr T &&value() && {
            if (is_err())
                throw bad_result_access("called value() on an Err Result");
            return std::get<0>(std::move(data));
        }

        constexpr T &operator*() & {
            return std::get<0>(data);
        }

        constexpr const T &operator*() const & {
            return std::get<0>(data);
        }

        constexpr T &&operator*() && {
            return std::get<0>(std::move(data));
        }

        constexpr T *operator->() {
            return &std::get<0>(data);
        }

        constexpr const T *operator->() const {
            return &std::get<0>(data);
        }

        /// Access the error value; throws bad_result_access if this is a success.
        constexpr E &error() & {
            if (is_ok())
                throw bad_result_access("called error() on an Ok Result");
            return std::get<1>(data);
        }

        constexpr const E &error() const & {
            if (is_ok())
                throw bad_result_access("called error() on an Ok Result");
            return std::get<1>(data);
        }

        constexpr E &&error() && {
            if (is_ok())
                throw bad_result_access("called error() on an Ok Result");
            return std::get<1>(std::move(data));
        }

        /// Returns the success value or a default if this is an error.
        constexpr T value_or(T &&default_val) const & {
            if (is_ok())
                return std::get<0>(data);
            return std::forward<T>(default_val);
        }

        constexpr T value_or(T &&default_val) && {
            if (is_ok())
                return std::get<0>(std::move(data));
            return std::forward<T>(default_val);
        }

        /// Monadic and_then: applies f (T -> Result<U, E>) if this is Ok.
        template <typename F>
        constexpr auto and_then(F &&f) & -> std::invoke_result_t<F, T &> {
            using Ret = std::invoke_result_t<F, T &>;
            if (is_ok())
                return std::forward<F>(f)(std::get<0>(data));
            return Ret::err(std::get<1>(data));
        }

        template <typename F>
        constexpr auto and_then(F &&f) const & -> std::invoke_result_t<F, const T &> {
            using Ret = std::invoke_result_t<F, const T &>;
            if (is_ok())
                return std::forward<F>(f)(std::get<0>(data));
            return Ret::err(std::get<1>(data));
        }

        template <typename F>
        constexpr auto and_then(F &&f) && -> std::invoke_result_t<F, T &&> {
            using Ret = std::invoke_result_t<F, T &&>;
            if (is_ok())
                return std::forward<F>(f)(std::get<0>(std::move(data)));
            return Ret::err(std::get<1>(std::move(data)));
        }

        /// Monadic transform: applies f (T -> U) if this is Ok, returning Result<U, E>.
        template <typename F>
        constexpr auto transform(F &&f) const & -> Result<std::invoke_result_t<F, const T &>, E> {
            using U = std::invoke_result_t<F, const T &>;
            if (is_ok())
                return Result<U, E>::ok(std::forward<F>(f)(std::get<0>(data)));
            return Result<U, E>::err(std::get<1>(data));
        }

        template <typename F>
        constexpr auto transform(F &&f) && -> Result<std::invoke_result_t<F, T &&>, E> {
            using U = std::invoke_result_t<F, T &&>;
            if (is_ok())
                return Result<U, E>::ok(std::forward<F>(f)(std::get<0>(std::move(data))));
            return Result<U, E>::err(std::get<1>(std::move(data)));
        }

        /// map_err: applies f (E -> F) if this is Err, returning Result<T, F>.
        template <typename G>
        constexpr auto map_err(G &&g) const & -> Result<T, std::invoke_result_t<G, const E &>> {
            using F2 = std::invoke_result_t<G, const E &>;
            if (is_err())
                return Result<T, F2>::err(std::forward<G>(g)(std::get<1>(data)));
            return Result<T, F2>::ok(std::get<0>(data));
        }

        template <typename G>
        constexpr auto map_err(G &&g) && -> Result<T, std::invoke_result_t<G, E &&>> {
            using F2 = std::invoke_result_t<G, E &&>;
            if (is_err())
                return Result<T, F2>::err(std::forward<G>(g)(std::get<1>(std::move(data))));
            return Result<T, F2>::ok(std::get<0>(std::move(data)));
        }

        /// or_else: calls f (E -> Result<T, E2>) if this is an Err.
        template <typename G>
        constexpr auto or_else(G &&g) const & -> std::invoke_result_t<G, const E &> {
            using Ret = std::invoke_result_t<G, const E &>;
            if (is_err())
                return std::forward<G>(g)(std::get<1>(data));
            return Ret::ok(std::get<0>(data));
        }

        template <typename G>
        constexpr auto or_else(G &&g) && -> std::invoke_result_t<G, E &&> {
            using Ret = std::invoke_result_t<G, E &&>;
            if (is_err())
                return std::forward<G>(g)(std::get<1>(std::move(data)));
            return Ret::ok(std::get<0>(std::move(data)));
        }

    private:
        template <std::size_t I, typename U>
        constexpr Result(std::in_place_index_t<I> tag, U &&v)
            : data(tag, std::forward<U>(v)) {}
    };

    /// Helper tag to construct a Result in the error state.
    template <typename E = std::string>
    struct Err {
        E value;

        constexpr explicit Err(E v)
            : value(std::move(v)) {}

        template <typename T>
        constexpr operator Result<T, E>() && {
            return Result<T, E>::err(std::move(value));
        }
    };

    template <typename E>
    Err(E) -> Err<E>;

    /// Helper tag to construct a Result in the success state.
    template <typename T>
    struct Ok {
        T value;

        constexpr explicit Ok(T v)
            : value(std::move(v)) {}

        template <typename E>
        constexpr operator Result<T, E>() && {
            return Result<T, E>::ok(std::move(value));
        }
    };

    template <typename T>
    Ok(T) -> Ok<T>;
} // namespace cpx

#endif
