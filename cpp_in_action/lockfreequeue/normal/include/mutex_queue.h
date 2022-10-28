#pragma once

#include "queue_node.h"

#include <condition_variable>
#include <mutex>

/**
 * @brief 基于链表和互斥锁实现的线程安全队列
 *
 */
class MutexQueue {
public:
    MutexQueue();
    bool enqueue(int val);
    int dequeue();
    ~MutexQueue();

private:
    int max_size = 100000;
    int queue_size;
    QueueNode* tail;
    QueueNode* head;
    std::mutex mtx;
    std::mutex producer_count_mtx;
    std::mutex consumer_count_mtx;

    std::condition_variable not_full;
    std::condition_variable not_empty;
};
