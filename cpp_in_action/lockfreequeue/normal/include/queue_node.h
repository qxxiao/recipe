#pragma once

#include <memory>

class QueueNode {
public:
    int val;
    QueueNode* next;
    QueueNode(int val) : val(val) {
        next = NULL;
    }
};
