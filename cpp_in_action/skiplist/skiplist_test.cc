#include "skiplist.h"

#include <iostream>
#include <random>

int main() {
    CSkipList skipList;
    /// 插入原始值
    for (int i = 1; i < 100; i++) {
        if ((i % 3) == 0) {
            skipList.Insert(i);
        }
    }
    for (int i = 1; i < 100; i++) {
        if ((i % 3) == 1) {
            skipList.Insert(i);
        }
    }
    skipList.PrintAll();

    std::cout << std::endl;
    /// 打印所有等级结构
    skipList.PrintAll(-1);
    /// 查找
    std::cout << std::endl;
    CNode *lpNode = skipList.Find(27);
    if (NULL != lpNode) {
        std::cout << "查找值为27的节点,找到该节点,节点值:" << lpNode->GetData() << std::endl;
    }
    else {
        std::cout << "查找值为27的节点,未找到该节点" << std::endl;
    }
    /// 删除
    std::cout << std::endl;
    int ret = skipList.Delete(46);
    if (0 == ret) {
        std::cout << "查找值为46的节点,找到该节点,并删除成功" << std::endl;
    }
    else {
        std::cout << "查找值为46的节点,找到该节点,删除失败" << std::endl;
    }
    std::cout << std::endl;
    //打印所有等级结构
    skipList.PrintAll(-1);
    std::cin.ignore();
    return 0;
}