#include "threadpool.h"

#include <future>
#include <iostream>
#include <unistd.h>
#include <vector>

int main() {

    // 注意缓存够的情况，不会创建新的cache线程
    ThreadPool pool(2, 10, 20);
    std::vector<std::future<int>> results;
    std::vector<int> res;
    std::cout << std::endl;

    // 添加100个任务
    auto start = std::chrono::steady_clock::now();
    for (int i = 1; i <= 100; ++i) {
        auto futureRes = pool.addTask([i] {
            volatile int sum = 0;
            for (int x = 0; x < 100; x++)
                sum += x;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return i;
        });
        if (!futureRes.valid()) {
            // std::cout << "invalid future" << std::endl;
            // std::cout << "add again:" << i << std::endl;
            i--;
        }
        else {
            results.push_back(std::move(futureRes));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto &result : results) {
        if (result.valid()) {
            res.push_back(result.get());
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms"
              << std::endl;

    std::cout << std::endl;
    std::cout << "result size: " << res.size() << std::endl;
    sort(res.begin(), res.end());
    for (int i = 0; i < res.size(); i++) {
        std::cout << res[i] << std::endl;
    }

    return 0;
}