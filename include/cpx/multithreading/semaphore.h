#ifndef CPX_MULTITHREADING_SEMAPHORE
#define CPX_MULTITHREADING_SEMAPHORE

#include "cpx/nomodule.h"
#include <condition_variable>
#include <mutex>

namespace cpx::multithreading {
    CPX_EXPORT class Semaphore {
    public:
        explicit Semaphore(int count = 0)
            : count(count) {}

        void acquire() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]() { return count > 0; });
            --count;
        }

        void release() {
            {
                std::lock_guard<std::mutex> lock(mutex);
                ++count;
            }
            cv.notify_one();
        }

    protected:
        std::mutex              mutex;
        std::condition_variable cv;
        int                     count;
    };
} // namespace cpx::multithreading

#endif
