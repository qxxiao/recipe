// * 单例模式 Lazy Singleton
#include <iostream>
#include <memory>
#include <mutex>

class Singleton {
public:
    // 禁止拷贝和赋值
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

public:
    static std::unique_ptr<Singleton> &GetInstance();
    static std::mutex m_mutex;
    ~Singleton();

private:
    Singleton();
    static std::unique_ptr<Singleton> instance;
};
