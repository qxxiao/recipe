#include "routineInfo.hpp"

#include <climits>
#include <cstring>
#include <iostream>
#include <list>

// 全局的协程列表routines
std::list<RoutineInfo *> InitRoutines() {
    std::list<RoutineInfo *> list;
    // 作为main协程状态
    RoutineInfo *main = new RoutineInfo(0);
    list.push_back(main);
    return list;
}
std::list<RoutineInfo *> routines = InitRoutines();

// 创建协程
// void *stackBackup = nullptr;
int createCoroutine(RoutineHandler handler, void *param) {
    // 16k+8k
    RoutineInfo *info = new RoutineInfo(PTHREAD_STACK_MIN + 0x4000);

    info->param = param;
    info->handler = handler;

    // 在实际线程中创建协程，并设置协程的状态退出
    pthread_t thread;
    int ret = pthread_create(&thread, &(info->attr), coroutineStart, info);
    void *status;
    pthread_join(thread, &status);

    // memcpy(info->stackbase, stackBackup, info->stacksize);
    // delete stackBackup;
    // free(stackBackup);
    // stackBackup = nullptr;

    routines.push_back(info); // add the routine to the end of the list
    return 0;
}

// call by CreateCoroutine
// 保存寄存器状态，复制协程的栈
void *coroutineStart(void *pRoutineInfo) {
    RoutineInfo &info = *(RoutineInfo *)pRoutineInfo;
    // 如果第一次调用返回0，保存寄存器状态
    // 如果调用longjmp返回，此时返回1继续执行下面的实际代码handler
    if (!setjmp(info.buf)) {
        // back up the stack, and then exit
        // stackBackup = realloc(stackBackup, info.stacksize);
        // memcpy(stackBackup, info.stackbase, info.stacksize);
        // exit会损坏栈的内容吗？
        pthread_exit(NULL);
        return (void *)0;
    }
    info.ret = info.handler(info.param);
    info.stopped = true;
    yield(); // never return
    return (void *)0;
}

// switch to another routine
std::list<RoutineInfo *> stoppedRoutines = std::list<RoutineInfo *>();
void yield() {
    RoutineInfo *current = routines.front();
    routines.pop_front();

    // 当前协程已经停止，加入到停止列表中
    std::cout << "current->stopped: " << current->stopped << std::endl;
    std::cout << "current->id: " << current->stackbase << " " << current->stacksize << std::endl;
    if (current->stopped) {
        // The stack is stored in the RoutineInfo object,
        // delete the object later
        stoppedRoutines.push_back(current);
        longjmp((*routines.begin())->buf, 1);
    }

    // adjust the routines to the end of list
    routines.push_back(current);

    // 需要切换本协程的栈到下一个协程的栈，将当前的状态保存到当前协程的buf中
    // main初始主协程执行流，只需要一个buf保持状态，自己有在加载时分配了栈
    if (!setjmp(current->buf)) {
        // RoutineInfo *next = *routines.begin();
        longjmp(routines.front()->buf, 1);
    }

    // 每一次切换回来(就是上面setjmp处并返回1)都需要检查是否有已经停止的协程，如果有则删除
    if (stoppedRoutines.size()) {
        delete stoppedRoutines.front();
        stoppedRoutines.pop_front();
    }
}
