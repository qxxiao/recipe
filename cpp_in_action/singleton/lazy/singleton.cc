#include "singleton.h"

std::unique_ptr<Singleton> Singleton::instance = nullptr;

std::unique_ptr<Singleton>& Singleton::GetInstance() {
    if (instance == nullptr) {
        instance = std::unique_ptr<Singleton>(new Singleton());
    }
    return instance;
}

// Double-Checked Locking Pattern(DCLP)
// std::unique_ptr<Singleton>& Singleton::GetInstance() {
//     if (instance == nullptr) {
//         std::lock_guard<std::mutex> lock(mutex);
//         if (instance == nullptr) {
//             instance = std::unique_ptr<Singleton>(new Singleton());
//         }
//     }
//     return instance;
// }

Singleton::Singleton() {
    std::cout << "singleton ctor" << std::endl;
}
Singleton::~Singleton() {
    std::cout << "singleton dtor" << std::endl;
}