#include <memory>

class Singleton {
public:
    // 禁止拷贝和赋值
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

    static Singleton &GetInstance();

private:
    Singleton();
    ~Singleton();
    // static Singleton instance; // use local static variable
};
