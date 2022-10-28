#include <pthread.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>

using RoutineHandler = void *(*)(void *);

/**
 * @brief TPC结构，用来描述协程的上下文
 *
 */
struct RoutineInfo {
    void *param;
    RoutineHandler handler;
    void *ret;
    bool stopped;
    // 使用setjump来保存当前协程的状态---寄存器状态
    jmp_buf buf;
    // 模拟的用户栈
    void *stackbase;
    size_t stacksize;

    pthread_attr_t attr; // 协程属性,使用栈的属性

    // size: the stack size
    RoutineInfo(size_t size) {
        param = nullptr;
        handler = nullptr;
        ret = nullptr;
        stopped = false;
        // !栈最小为16KB，并且是页面(通常为4KB)对齐, 使用posix_memalign
        posix_memalign(&stackbase, getpagesize(), size);
        stacksize = size;

        pthread_attr_init(&attr);
        //调用 pthread_attr_setstack 函数来显式地声明使用的程序栈
        if (stacksize)
            pthread_attr_setstack(&attr, stackbase, stacksize);
    }

    ~RoutineInfo() {
        pthread_attr_destroy(&attr);
        free(stackbase);
    }
};

int createCoroutine(RoutineHandler handler, void *param);
// CoroutinneStart 函数
void *coroutineStart(void *pRoutineInfo);
// Switch 函数
void yield();
