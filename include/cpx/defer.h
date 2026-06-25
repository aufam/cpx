#ifndef CPX_DEFER_H
#define CPX_DEFER_H

#include <cpx/nomodule.h>
#include <utility>
#include <type_traits>

namespace cpx {
    CPX_EXPORT template <typename F>
    class defer {
    public:
        static_assert(std::is_invocable_v<F>, "F must be invocable");

        defer(F fn)
            : fn(std::move(fn)) {}

        ~defer() {
            fn();
        }

        F fn;
    };
} // namespace cpx

#endif
