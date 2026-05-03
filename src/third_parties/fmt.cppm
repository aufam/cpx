module;

#include <cpx/tag_info.h>
#include <optional>
#include <variant>

#if __has_include(<boost/pfr.hpp>)
#    include <boost/pfr.hpp>
#endif

#if __has_include(<magic_enum/magic_enum.hpp>)
#    include <magic_enum/magic_enum.hpp>
#endif

#define FMT_RANGES_H_
#define FMT_CHRONO_H_

export module cpx.fmt;
export import fmt;
import cpx;

#include "cpx/fmt.h"
export {
    template <typename T>
    struct fmt::formatter<::cpx::Tag<T>, char, std::enable_if_t<::fmt::is_formattable<T, char>::value>>;

    template <typename T>
    struct fmt::formatter<std::optional<T>, char, std::enable_if_t<::fmt::is_formattable<T, char>::value>>;

    template <typename... T>
    struct fmt::formatter<std::variant<T...>, char, std::enable_if_t<(::fmt::is_formattable<T, char>::value && ...)>>;

#ifdef BOOST_PFR_HPP
    template <typename S>
    struct fmt::formatter<S, char, std::enable_if_t<std::is_aggregate_v<S> && !std::is_same_v<S, std::tm>>>;
#endif

#ifdef NEARGYE_MAGIC_ENUM_HPP
    template <typename S>
    struct fmt::formatter<S, char, std::enable_if_t<std::is_enum_v<S>>>;
#endif
}
