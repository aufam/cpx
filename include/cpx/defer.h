#ifndef CPX_DEFER_H
#define CPX_DEFER_H

#include <utility>

namespace cpx {
    template <typename F>
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
