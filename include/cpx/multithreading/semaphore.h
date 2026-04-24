#ifndef CPX_MULTITHREADING_SEMAPHORE
#define CPX_MULTITHREADING_SEMAPHORE

#include <condition_variable>
#include <mutex>

namespace cpx::multithreading {
    class Semaphore {
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
