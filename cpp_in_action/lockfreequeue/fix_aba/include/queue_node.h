

#include "double_cas.h"

#include <memory>

class QueueNode {
public:
    typedef DPointer<QueueNode, sizeof(size_t)> Pointer;
    int val;
    Pointer next;
    QueueNode() : next(NULL) {}
    QueueNode(int val) : val(val), next(NULL) {}
};
