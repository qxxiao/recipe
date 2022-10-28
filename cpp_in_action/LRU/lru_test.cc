#include "lru.h"

#include <iostream>
using std::cout, std::endl;

int main() {
    LRUCache cache(1);

    cache.put(2, 1);
    cout << cache.get(2) << endl;
    cache.put(3, 2);
    cout << cache.get(2) << endl;
    cout << cache.get(3) << endl;
    return 0;
}