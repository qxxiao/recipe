#include "lockfreequeue.h"

LockFreeQueue::LockFreeQueue() {
    queue_size = 0;
    tail = new QueueNode(-1); // dummy node
    head = tail;
}

LockFreeQueue::~LockFreeQueue() {}

bool LockFreeQueue::enqueue(int val) {
    QueueNode* cur_node;
    QueueNode* node = new QueueNode(val);
    while (true) {
        cur_node = tail; // 读取尾节点
        if (__sync_bool_compare_and_swap(&(cur_node->next), nullptr, node)) {
            break;
        }
        // 加入不成功，说明这时候有其他线程抢先往里面插入了一个节点，那么就把当前节点的位置更新为尾节点，
        // 再次进入循环直到能正确更新
        else {
            // 这里主要是为了防止（插入成功的线程在下面的更新tail时挂掉等原因时不会在原来tail死循环）
            // 如果失败，则tail已经被正确更新，不需要再更新
            __sync_bool_compare_and_swap(&tail, cur_node, cur_node->next);
        }
    }
    // 不需要循环，原因是：如果当前线程将节点已经加进去了的话，那么其他所有线程的操作都会失败，
    // 只有当前线程更新尾节点完成后，其他线程的第二个CAS操作才能成功
    __sync_bool_compare_and_swap(&tail, cur_node, node);
    return true;
}

int LockFreeQueue::dequeue() {
    QueueNode* cur_node;
    int val;
    // 为了避免同时修改 head和tail（只有一个节点）
    // 将 后面一个节点作为 dummy 节点，删除原来的 dummy 节点
    while (true) {
        cur_node = head;
        if (cur_node->next == nullptr) { // 队列此刻为空
            return -1;
        }
        // head 向后移动一次
        if (__sync_bool_compare_and_swap(&head, cur_node, cur_node->next)) {
            break;
        }
    }
    // 删除原来的 dummy 节点
    // 虽然 head 可能继续移动，但是链式关系还是对的
    val = cur_node->next->val;
    delete cur_node;
    return val;
}