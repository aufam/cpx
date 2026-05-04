module;

#include <cpx/multithreading/channel.h>
#include <cpx/multithreading/queue.h>
#include <cpx/multithreading/semaphore.h>

export module cpx.multithreading;
import cpx;

export namespace cpx::multithreading {
    using ::cpx::multithreading::Channel;
    using ::cpx::multithreading::Queue;
    using ::cpx::multithreading::Semaphore;
} // namespace cpx::multithreading
