#include "singleton.h"

#include <memory>

using namespace std;

int main() {
    cout << "entry main......" << endl;
    unique_ptr<Singleton>& s = Singleton::GetInstance();
    return 0;
}
