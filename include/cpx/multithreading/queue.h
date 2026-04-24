#ifndef CPX_MULTITHREADING_QUEUE
#define CPX_MULTITHREADING_QUEUE

#include <condition_variable>
#include <optional>
#include <queue>
#include <vector>
#include <mutex>

namespace cpx::multithreading {
    template <typename T>
    class Queue {
    public:
        Queue() = default;

        ~Queue() {
            stop();
        }

        // push an item. Returns true if the item was accepted, false if the queue is stopped.
        bool push(T item) {
            std::unique_lock<std::mutex> lock(mtx);
            if (stopped)
                return false;

            queue.push(std::move(item));
            lock.unlock();
            cv.notify_all();
            return true;
        }

        // pop all items. Blocks until an item is available or the queue is stopped.
        std::vector<T> pop() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return stopped || !queue.empty(); });

            std::vector<T> batch;
            batch.reserve(queue.size());
            while (!queue.empty()) {
                batch.emplace_back(std::move(queue.front()));
                queue.pop();
            }

            return batch;
        }

        std::optional<T> pop_one() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return stopped || !queue.empty(); });

            if (queue.empty())
                return std::nullopt;

            T item = std::move(queue.front());
            queue.pop();

            return std::move(item);
        }

        // stop accepting new items, wake waiting pop() calls
        void stop() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                stopped = true;
            }
            cv.notify_all();
        }

        void clear() {
            std::lock_guard<std::mutex> lock(mtx);
            while (!queue.empty())
                queue.pop();
        }

        bool is_stopped() const noexcept {
            std::lock_guard<std::mutex> lock(mtx);
            return stopped;
        }

        bool is_empty() const noexcept {
            std::lock_guard<std::mutex> lock(mtx);
            return queue.empty();
        }

        size_t queued_size() const {
            std::lock_guard<std::mutex> lock(mtx);
            return queue.size();
        }

    private:
        mutable std::mutex      mtx;
        std::condition_variable cv;
        std::queue<T>           queue;
        bool                    stopped{};
    };
} // namespace cpx::multithreading

#endif
