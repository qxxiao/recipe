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

using namespace std::chrono_literals;

class ThreadPool {
public:
    enum class ThreadState {
        RUNNING = 1,
        IDLE = 2,
        STOP = 3,
    };
    // 也可以不区分核心线程
    enum class ThreadType {
        CORE = 1,
        NORMAL = 2,
    };

    using ThreadPtr = std::shared_ptr<std::thread>;
    using ThreadId = std::atomic<int>;
    using ThreadStateAtomic = std::atomic<ThreadState>;
    using ThreadTypeAtomic = std::atomic<ThreadType>;

    struct ThreadWrapper {
        ThreadPtr ptr;
        ThreadId id;
        ThreadStateAtomic state;
        ThreadTypeAtomic type;

        ThreadWrapper(ThreadPtr ptr = nullptr) : ptr(ptr) {
            id = 0;
            // state = ThreadState::RUNNING;
            // type = ThreadType::CORE;
        }
    };

    //* coreThreads: 核心线程数(大于0) min(coreThreads,2*hardware_concurrency)
    //* maxThreads: 最大线程数 min(maxThreads, 4*coreThreads)
    //* maxTasks: 最大任务数 default: 5*maxThreads
    //* timeout: 线程空闲超时时间 default: 5000ms
    ThreadPool(size_t coreThreads, size_t maxThreads, size_t maxTasks = 0,
               std::chrono::milliseconds timeout = 5000ms);

    template <typename F, typename... Args>
    auto addTask(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
    void shutdown();      //缓存的任务不会被执行
    void shutdownDelay(); //等待队列中的任务执行完
    ~ThreadPool();

    // Get methods
    int getNumThreads() const;
    int getNumIdleThreads() const;
    int getNumRemainedTasks() const;
    int getNumFinishedTasks() const;

private:
    int getNextThreadId();
    void createThread(std::function<void()>);
    void deleteThread(std::shared_ptr<ThreadWrapper> pWrapper);
    void innerShutDown(bool);

private:
    // basic parameters
    size_t coreThreads;                // 核心线程数
    size_t maxThreads;                 // 最大线程数
    size_t maxTasks;                   // 最大任务数
    std::chrono::milliseconds timeout; // 线程等待超时时间

    // thread pool state
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::condition_variable cond;

    std::list<std::shared_ptr<ThreadWrapper>> workers; // 线程
    std::mutex workers_mutex;
    std::atomic_int threadIDInit;      // 线程ID初始化
    std::atomic_int num_threads;       // 线程数
    std::atomic_int num_idle_threads;  // 当前空闲线程数
    std::atomic_int num_remainedTasks; // 当前剩余任务数
    std::atomic_int num_finishedTasks; // 当前完成任务数

    // 结束线程池
    std::atomic_bool is_shutdown;      // 当前工作线程完成退出
    std::atomic_bool is_shutdownDelay; // 等待任务队列为空
};

// return a future object
// if the task is rejected, the future object is invalid
template <typename F, typename... Args>
auto ThreadPool::addTask(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    // decltype(f(args...))
    using return_type = typename std::result_of<F(Args...)>::type;

// log
#ifdef DEBUG
    std::cout << "num_threads: " << num_threads
              << " leaft tasks: " << tasks.size() << " = "
              << this->num_remainedTasks << std::endl;
#endif

    // threadpool is shutdown, not allowed to add task
    if (is_shutdown || is_shutdownDelay)
        throw std::runtime_error("threadpool is stopped");

    auto ptask = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = ptask->get_future();

    { // 没有达到core_theads的数量，则创建新线程
        if (num_threads < coreThreads) {
            createThread([ptask]() { (*ptask)(); });
            return res;
        }
    }
    { // 等待队列没有满时，添加任务
        std::lock_guard<std::mutex> lk(tasks_mutex);
        if (tasks.size() < maxTasks) {
            tasks.emplace([ptask]() { (*ptask)(); });
            this->num_remainedTasks++;
            cond.notify_one();
            return res;
        }
    }
    {
        // taskqueue is full, not reached maxthreads, create a new thread
        if (num_threads < maxThreads) {
            createThread([ptask]() { (*ptask)(); });
            return res;
        }
    }
    // taskqueue is full, and up to maxthreads reutrn empty future
    return std::future<return_type>();
}