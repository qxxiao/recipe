#include "singleton.h"

#include <iostream>

using namespace std;

int main() {

    cout << "entry main......" << endl;
    Singleton& s = Singleton::GetInstance();
    Singleton& s2 = Singleton::GetInstance();
    cout << "s == s2: " << (&s == &s2) << endl;
    // With the modern C++ it is allowed to declare even private destructors for
    // statically constructed objects. Here is my code snippet for Singleton
    return 0;
}