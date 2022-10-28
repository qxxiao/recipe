#pragma once

#include "queue_node.h"

/**
 * @brief 基于链表的无锁队列实现
 *
 */
class LockFreeQueue {
public:
    LockFreeQueue();
    bool enqueue(int val);
    int dequeue();
    ~LockFreeQueue();

private:
    // 不使用锁，因此需要保证原子性
    // 使用 std::atomic<int> 保证原子性或者直接使用 __sync_fetch_and_add
    // 如果达到最大值，就不再入队
    int queue_size; // 暂时未使用
    QueueNode* tail;
    QueueNode* head;
};
