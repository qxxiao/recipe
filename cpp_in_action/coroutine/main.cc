#include "routineInfo.hpp"

#include <iostream>
#include <unistd.h>
using namespace std;

#include <sys/wait.h>

void *foo(void *) {
    for (int i = 0; i < 2; ++i) {
        cout << "foo: " << i << endl;
        sleep(1);
        yield();
    }
    return nullptr;
}

int main() {
    createCoroutine(foo, NULL);
    for (int i = 0; i < 6; ++i) {
        cout << "main: " << i << endl;
        sleep(1);
        yield();
    }
}