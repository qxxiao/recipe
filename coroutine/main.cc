#include <unistd.h>

#include <iostream>

#include "routineInfo.hpp"
using namespace std;

#include <sys/wait.h>

void* foo(void*) {
    for (int i = 0; i < 2; ++i) {
        cout << "foo: " << i << endl;
        sleep(1);
        Switch();
    }
    return nullptr;
}

int main() {
    CreateCoroutine(foo, NULL);
    for (int i = 0; i < 6; ++i) {
        cout << "main: " << i << endl;
        sleep(1);
        Switch();
    }
}