#include <pthread.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>

using RoutineHandler = void *(*)(void *);

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

    pthread_attr_t attr;  // 协程属性,使用栈的属性

    // size: the stack size
    RoutineInfo(size_t size) {
        param = nullptr;
        handler = nullptr;
        ret = nullptr;
        stopped = false;
        // !栈最小为16KB，并且是页面(通常为4KB)对齐, 主要直接使用malloc分配的地址，使用posix_memalign
        // stackbase = malloc(size);
        posix_memalign(&stackbase, getpagesize(), size);
        stacksize = size;

        pthread_attr_init(&attr);
        //调用 pthread_attr_setstack 函数来显式地声明使用的程序栈
        if (stacksize) pthread_attr_setstack(&attr, stackbase, stacksize);
    }

    ~RoutineInfo() {
        pthread_attr_destroy(&attr);
        free(stackbase);
    }
};

int CreateCoroutine(RoutineHandler handler, void *param);
// CoroutinneStart 函数
void *CoroutineStart(void *pRoutineInfo);
// Switch 函数
void Switch();
