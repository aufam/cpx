#ifndef CPX_TUPLE_H
#define CPX_TUPLE_H

#include <tuple>
#include <type_traits>

namespace cpx {
    template <typename T>
    struct is_tuple : std::false_type {};

    template <typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};

    template <typename T>
    constexpr bool is_tuple_v = is_tuple<T>::value;


    template <typename F, typename Tuple>
    struct is_invocable_with_tuple;

    template <typename F, typename... Ts>
    struct is_invocable_with_tuple<F, std::tuple<Ts...>> : std::is_invocable<F, Ts...> {};

    template <typename F, typename Tuple>
    inline constexpr bool is_invocable_with_tuple_v = is_invocable_with_tuple<F, Tuple>::value;


    template <typename Tuple, template <typename> typename Pred>
    struct filter_tuple;

    template <typename Tuple, template <typename> typename Pred>
    using filter_tuple_t = typename filter_tuple<Tuple, Pred>::type;

    template <typename Tuple, template <typename> typename Pred>
    struct apply_tuple;

    template <typename Tuple, template <typename> typename Pred>
    using apply_tuple_t = typename apply_tuple<Tuple, Pred>::type;

    template <template <typename> typename Pred, typename... Ts>
    struct filter_tuple<std::tuple<Ts...>, Pred> {
    private:
        template <typename T>
        using keep = std::bool_constant<Pred<T>::value>;

        template <typename, typename>
        struct append;

        template <typename... Us, typename T>
        struct append<std::tuple<Us...>, T> {
            using type = std::tuple<Us..., T>;
        };

        template <typename Result, typename... Rest>
        struct impl;

        template <typename Result>
        struct impl<Result> {
            using type = Result;
        };

        template <typename Result, typename T, typename... Rest>
        struct impl<Result, T, Rest...> {
            using next = std::conditional_t<Pred<T>::value, typename append<Result, T>::type, Result>;
            using type = typename impl<next, Rest...>::type;
        };

    public:
        using type = typename impl<std::tuple<>, Ts...>::type;
    };

    template <template <typename> typename Pred, typename... Ts>
    struct apply_tuple<std::tuple<Ts...>, Pred> {
        using type = std::tuple<typename Pred<Ts>::type...>;
    };


    template <typename Tuple, typename F>
    constexpr void tuple_for_each(Tuple &&tpl, F &&fn) {
        std::apply(
            [&](auto &...item) {
                std::size_t i = 0;
                (fn(item, i++), ...);
            },
            std::forward<Tuple>(tpl)
        );
    }

    template <template <typename> class Pred, typename T>
    constexpr auto tie_if_one(T &v) {
        if constexpr (Pred<std::remove_reference_t<T>>::value) {
            return std::tie(v);
        } else {
            return std::tuple<>();
        }
    }

    template <template <typename> class Pred, typename... Ts>
    constexpr auto tie_if(Ts &...vs) {
        return std::tuple_cat(tie_if_one<Pred>(vs)...);
    }
} // namespace cpx

#endif
