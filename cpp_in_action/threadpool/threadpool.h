#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

#if defined(DEBUG)
#    include <iostream>
#endif

class ThreadPool {
public:
    // thread state
    enum class ThreadState {
        RUNNING = 1,
        IDLE = 2,
        STOP = 3,
    };
    // thread type
    enum class ThreadType {
        CORE = 1,
        NORMAL = 2,
    };

    using ThreadId = std::atomic<int>;
    using ThreadStateAtomic = std::atomic<ThreadState>;
    using ThreadTypeAtomic = std::atomic<ThreadType>;

    struct ThreadWrapper {
        std::thread t;
        ThreadId id;
        ThreadStateAtomic state;
        ThreadTypeAtomic type;

        ThreadWrapper() {}
    };

    //* coreThreads: 核心线程数(大于0, 否则 coreNums)
    //* maxThreads: 最大线程数 min(maxThreads)
    //* maxTasks: 最大任务数(0表示不限制任务数)
    //* timeout: 非核心线程空闲超时时间 default: 5000ms
    ThreadPool(size_t coreThreads, size_t maxThreads, size_t maxTasks = 0,
               std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    template <typename F, typename... Args>
    auto addTask(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;
    void shutdown();      //缓存的任务不会被执行
    void shutdownDelay(); //等待队列中的任务执行完
    ~ThreadPool();

    // Get methods
    inline int getNumThreads() const;
    inline int getNumIdleThreads() const;
    inline int getNumRemainedTasks() const;
    inline int getNumFinishedTasks() const;

private:
    inline int getNextThreadId();
    void createThread(std::function<void()>);
    void deleteThread(std::shared_ptr<ThreadWrapper> pWrapper);
    void innerShutDown(bool);

private:
    size_t coreThreads;                // 核心线程数
    size_t maxThreads;                 // 最大线程数
    size_t maxTasks;                   // 最大任务数
    std::chrono::milliseconds timeout; // 非核心线程等待超时时间

    // thread pool state
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::condition_variable cond;

    std::list<std::shared_ptr<ThreadWrapper>> workers; // 线程
    std::mutex workers_mutex;

    std::atomic_int threadIDInit;      // 线程ID初始化
    std::atomic_int num_threads;       // 当前线程数
    std::atomic_int num_idle_threads;  // 当前空闲线程数
    std::atomic_int num_remainedTasks; // 当前剩余任务数
    std::atomic_int num_finishedTasks; // 当前完成任务数

    // 结束线程池
    std::atomic_bool is_shutdown;      // 立即退出，当前工作线程完成退出
    std::atomic_bool is_shutdownDelay; // 等待任务队列为空退出
};

// return a future
// if the task is rejected, the future is invalid
template <typename F, typename... Args>
auto ThreadPool::addTask(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type> {
    // decltype(f(args...))
    using return_type = typename std::result_of<F(Args...)>::type;
#ifdef DEBUG
    std::cout << "num_threads: " << num_threads << " leaft tasks: " << tasks.size() << " = " << this->num_remainedTasks
              << std::endl;
#endif

    // threadpool is shutdown, not allowed to add task
    if (is_shutdown || is_shutdownDelay)
        throw std::runtime_error("threadpool is stopped");

    auto ptask =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = ptask->get_future();

    { // thread num < core threads, create new thread and run task
        // !! not thread safe, but it's ok with main thread
        if (num_threads < coreThreads) {
            createThread([ptask]() { (*ptask)(); });
            return res;
        }
    }
    { // taskqueue is not full, add task to taskqueue
        std::lock_guard<std::mutex> lk(tasks_mutex);
        if (tasks.size() < maxTasks || !maxTasks) {
            tasks.emplace([ptask]() { (*ptask)(); });
            this->num_remainedTasks++;
            cond.notify_one();
            return res;
        }
    }
    { // taskqueue is full, but thread num < maxThreads, create new thread and run task
        // !! not thread safe, but it's ok with main thread
        if (num_threads < maxThreads) {
            createThread([ptask]() { (*ptask)(); });
            return res;
        }
    }
    // taskqueue is full, and up to maxthreads reutrn empty future
    return std::future<return_type>();
}