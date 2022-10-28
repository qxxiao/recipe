
#include "mutex_queue.h"
#ifdef DEBUG
#    include <iostream>
#endif

MutexQueue::MutexQueue() {
    queue_size = 0;
    tail = new QueueNode(-1); // dummy node
    head = tail;
}

MutexQueue::~MutexQueue() {}

bool MutexQueue::enqueue(int val) {
    QueueNode* node = new QueueNode(val);

    std::unique_lock<std::mutex> lock(mtx);
    while (queue_size == max_size) {
#ifdef DEBUG
        std::cout << "queue is full, wait for dequeue" << std::endl;
#endif
        not_full.wait(lock);
    }
    tail->next = node;
    tail = node;
    queue_size++;
    not_empty.notify_one();
    return true;
}

int MutexQueue::dequeue() {
    std::unique_lock<std::mutex> lock(mtx);
    while (queue_size == 0) {
#ifdef DEBUG
        std::cout << "queue is empty, wait for enqueue" << std::endl;
#endif
        not_empty.wait(lock);
    }
    QueueNode* node = head->next;
    head->next = node->next;
    if (node == tail) {
        tail = head;
    }
    int val = node->val;
    delete node;
    queue_size--;
    not_full.notify_one();
    return val;
}