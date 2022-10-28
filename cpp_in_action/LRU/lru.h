#include <unordered_map>

class LRUCache {
    struct Node {
        int key, val;
        Node *pre, *next;
        Node() : key(0), val(0), pre(nullptr), next(nullptr) {}
        Node(int key, int value) : key(key), val(value), pre(nullptr), next(nullptr) {}
    };
    int cap;
    std::unordered_map<int, Node*> hash;
    Node *head, *tail;

public:
    LRUCache(int capacity) {
        cap = capacity;
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->pre = head;
    }

    ~LRUCache() {
        Node* cur = head;
        while (cur) {
            Node* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
    }

    int get(int key) {
        // -1 means not found
        if (!hash.count(key))
            return -1;
        auto node = hash[key];
        moveToHead(node);
        return node->val;
    }
    // 如果存在也要移动到头部
    void put(int key, int value) {
        if (cap <= 0)
            return;
        if (hash.count(key)) {
            auto node = hash[key];
            moveToHead(node);
            node->val = value;
            return;
        }
        // if (hash.size() == cap)(cap>=1)
        if (hash.size() >= cap) {
            hash.erase(tail->pre->key);
            tail = tail->pre;
            delete tail->next;
            tail->next = nullptr;
        }
        insert(key, value);
    }

private:
    // 访问过的节点移动到头部
    void moveToHead(Node* node) {
        node->pre->next = node->next;
        node->next->pre = node->pre;
        node->next = head->next;
        node->next->pre = node;
        node->pre = head;
        head->next = node;
    }
    // 不存在的时候才插入
    void insert(int key, int value) {
        auto node = new Node(key, value);
        hash[key] = node;
        node->next = head->next;
        node->next->pre = node;
        head->next = node;
        node->pre = head;
    }
};