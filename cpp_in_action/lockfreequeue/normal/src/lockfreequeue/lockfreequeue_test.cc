#include "LockFreeQueue.h"

#ifdef DEBUG
#    include <iostream>
#endif
#include <thread>
#include <vector>

int thread_number;
int task_number; // 每个线程需要入队/出队的资源个数
LockFreeQueue* lfq;

void produce(int offset) {
    // 算上偏移量，保证不会出现重复
    for (int i = task_number * offset; i < task_number * (offset + 1); i++) {
#ifdef DEBUG
        printf("produce %d\n", i);
#endif
        lfq->enqueue(i);
    }
}

// 消费者，出队列操作
void consume() {
    // 出队列 task_number 次数
    for (int i = 0; i < task_number; i++) {
        int res = lfq->dequeue();
#ifdef DEBUG
        if (res > 0)
            printf("consume %d\n", res);
        else
            printf("Fail to consume!\n");
#endif
    }
}

int main(int argc, char** argv) {
    lfq = new LockFreeQueue;
    std::vector<std::thread> thread_vector1;
    std::vector<std::thread> thread_vector2;

    if (argc < 3) {
        thread_number = 10;
        task_number = 10000;
    }
    else {
        thread_number = atoi(argv[1]);
        task_number = atoi(argv[2]);
    }

    for (int i = 0; i < thread_number; i++) {
        thread_vector1.push_back(std::thread(produce, i));
        thread_vector2.push_back(std::thread(consume));
    }

    for (auto& thr1 : thread_vector1) {
        thr1.join();
    }

    for (auto& thr2 : thread_vector2) {
        thr2.join();
    }
    return 0;
}