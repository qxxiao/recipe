#include "singleton.h"

#include <iostream>

// in Singleton's scope
// With the modern C++ it is allowed to declare even private destructors for
// statically constructed objects.
// Singleton Singleton::instance; // 饿汉模式可以在这里初始化单例

// 静态局部变量存储在静态存储区,程序结束自动析构静态变量
//--------------------
// C++11规定了局部静态变量在多线程条件下的初始化行为，要求编译器需保证局部静态变量的安全性
// 使用局部静态变量可以调整为线程安全的懒汉模式
Singleton &Singleton::GetInstance() {
    static Singleton instance;
    return instance;
}

Singleton::Singleton() {
    std::cout << "singleton ctor" << std::endl;
}

Singleton::~Singleton() {
    std::cout << "singleton dtor" << std::endl;
}