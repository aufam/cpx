#ifndef CPX_MULTITHREADING_CHANNEL
#define CPX_MULTITHREADING_CHANNEL

#include <cpx/multithreading/semaphore.h>
#include <queue>
#include <future>

namespace cpx::multithreading {
    template <typename T>
    class Channel {

    public:
        static_assert(!std::is_reference_v<T>, "T must not be a reference type");

        explicit Channel(int n)
            : sem(n)
            , terminated(false) {}

        ~Channel() {
            while (!futures.empty()) {
                futures.front().get();
                futures.pop();
            }
        }

        template <typename F>
        void push(F &&f) {
            using R = std::invoke_result_t<F>;
            static_assert(std::is_same_v<R, T>, "Function must return T");

            if (terminated.load())
                return;

            sem.acquire();
            futures.push(std::async(std::launch::async, [this, func = std::forward<F>(f)]() -> T {
                SemaphoreReleaser _r(sem);
                return func();
            }));
        }

        template <typename F>
        void pop(F &&f) {
            static_assert(std::is_invocable_v<F, T>, "Function must accept T");

            f(futures.front().get());
            futures.pop();
        }

        bool empty() const {
            return futures.empty();
        }
        void terminate() {
            terminated.store(true);
        }

    protected:
        struct SemaphoreReleaser {
            Semaphore &sem;
            explicit SemaphoreReleaser(Semaphore &s)
                : sem(s) {}
            ~SemaphoreReleaser() {
                sem.release();
            }
        };

        std::queue<std::future<T>> futures;
        Semaphore                  sem;
        std::atomic_bool           terminated;
    };
} // namespace cpx::multithreading
#endif
