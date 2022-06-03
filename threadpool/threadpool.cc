#include "threadpool.h"

ThreadPool::ThreadPool(size_t coreThreads, size_t maxThreads, size_t maxTasks,
                       std::chrono::milliseconds timeout) {
    unsigned int coreNums = std::thread::hardware_concurrency();
    // not up to 16 and 1.5*coreNums
    coreThreads = std::max(1ul, coreThreads);
    this->coreThreads =
        coreNums == 0
            ? std::min(coreThreads, 16ul)
            : std::min(coreThreads, static_cast<size_t>(2 * coreNums));
    this->maxThreads = maxThreads <= coreThreads
                           ? this->coreThreads
                           : std::min(maxThreads, 4 * this->coreThreads);
    this->maxTasks = maxTasks <= 0 ? 5 * this->maxThreads : maxTasks;
    this->timeout = timeout;

    this->num_threads = 0;
    this->num_idle_threads = 0;
    this->threadIDInit = 0;
    this->num_remainedTasks = 0; // tasks size
    this->num_finishedTasks = 0;
    this->is_shutdown = false;
    this->is_shutdownDelay = false;
// log
#ifdef DEBUG
    std::cout << "hardware_concurrency: " << coreNums << std::endl
              << "coreThreads: " << this->coreThreads << std::endl
              << "maxThreads: " << this->maxThreads << std::endl
              << "maxTasks: " << this->maxTasks << std::endl
              << "timeout: " << this->timeout.count() << "ms" << std::endl;
#endif
}

int ThreadPool::getNextThreadId() {
    return threadIDInit++;
}

// create a new thread with a Task(or ==nullptr)
void ThreadPool::createThread(std::function<void()> ptask) {
    // assert(workers.size() < maxThreads);
    auto pthreadWrapper = std::make_shared<ThreadWrapper>();
    pthreadWrapper->id = getNextThreadId();
    pthreadWrapper->state = ThreadState::IDLE;
    pthreadWrapper->type = this->num_threads < this->coreThreads
                               ? ThreadType::CORE
                               : ThreadType::NORMAL;
    std::weak_ptr<ThreadWrapper> wpthreadWrapper(pthreadWrapper);
    auto func = [this, wpthreadWrapper, ptask] {
        // get the shared_ptr pwrapper
        auto pwrapper = wpthreadWrapper.lock();
        if (!pwrapper) {
            // log error
            return;
        }
        // if starting a thread with a init task
        if (ptask) {
            pwrapper->state = ThreadState::RUNNING;
            ptask();
            pwrapper->state = ThreadState::IDLE;
            this->num_finishedTasks++;
        }
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lk(tasks_mutex);
                // wait until there is a task or the threadpool is shutdown
                this->num_idle_threads++;
                bool notTimeout = true;
                // for core threads
                if (pwrapper->type == ThreadType::CORE) {
                    this->cond.wait(lk, [this] {
                        return this->is_shutdown || this->is_shutdownDelay
                               || !this->tasks.empty();
                    });
                }
                // for normal threads
                else {
                    notTimeout = this->cond.wait_for(lk, this->timeout, [this] {
                        return this->is_shutdown || this->is_shutdownDelay
                               || !this->tasks.empty();
                    });
                }
                this->num_idle_threads--;
                // for NORMAL Thread: timeout 并且条件不满足
                if (!notTimeout) {
                    pwrapper->state = ThreadState::STOP;
                    this->num_threads--;
                    break; // exit thread
                }
                // 响应线程池退出
                if (this->is_shutdown
                    || (this->is_shutdownDelay && this->tasks.empty())) {
                    pwrapper->state = ThreadState::STOP;
                    this->num_threads--;
                    break;
                }
                task = std::move(this->tasks.front());
                // log
#ifdef DEBUG
                std::cout << "num_thread: " << this->num_threads
                          << " remained tasks: " << this->num_remainedTasks
                          << std::endl;
                std::cout << "thread " << pwrapper->id << " get a queue task"
                          << std::endl;
#endif
                this->tasks.pop();
                this->num_remainedTasks--;
            }
            pwrapper->state = ThreadState::RUNNING;
            task();
            pwrapper->state = ThreadState::IDLE;
            this->num_finishedTasks++;
        }
    };
    // 线程实例化
    pthreadWrapper->ptr = std::make_shared<std::thread>(std::move(func));
    {
        // std::lock_guard<std::mutex> lk(workers_mutex);
        workers.emplace_back(std::move(pthreadWrapper));
    }
    this->num_threads++;
}

// void ThreadPool::deleteThread(std::shared_ptr<ThreadWrapper> pWrapper) {
//     {
//         std::lock_guard<std::mutex> lk(workers_mutex);
//         this->workers.remove(pWrapper);
//     }
// }

void ThreadPool::innerShutDown(bool now) {
    if (is_shutdown || is_shutdownDelay)
        return;
    if (now)
        is_shutdown = true;
    else
        is_shutdownDelay = true;
    // notify all threads
    cond.notify_all();
    // join all threads
    {
        // std::lock_guard<std::mutex> lk(workers_mutex);
        for (auto& worker : workers) {
            if (worker->ptr->joinable())
                worker->ptr->join();
        }
    }
}

void ThreadPool::shutdown() {
    innerShutDown(true);
}

void ThreadPool::shutdownDelay() {
    innerShutDown(false);
}

ThreadPool::~ThreadPool() {
    shutdownDelay();
}

// get number of threads
int ThreadPool::getNumThreads() const {
    return num_threads;
}
// get number of idle/waiting threads
int ThreadPool::getNumIdleThreads() const {
    return num_idle_threads;
}
// get number of remained tasks
int ThreadPool::getNumRemainedTasks() const {
    return num_remainedTasks;
}
// get number of finished tasks
int ThreadPool::getNumFinishedTasks() const {
    return num_finishedTasks;
}
