#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <random>
#include <string>
using namespace std;

int GetRandom();

/**
 *  https://github.com/wangzheng0822/algo/blob/master/c-cpp/17_skiplist/SkipList.cpp
 *  简单实现，只存储一个整数值
 *  跳表结构:
 *
 *  第K级          1                       9
 *  第K-1级        1          5            9
 *  第K-2级        1    3     5      7     9
 *  ...             ....
 *  第0级(原始链表) 1  2  3  4  5  6  7  8  9
 */

constexpr int MAX_LEVEL = 16;

/**
 * @brief 节点
 */
class CNode {
public:
    CNode();
    ~CNode();

    std::string toString();
    /**
     * @brief 获取索引链表
     */
    CNode **GetIdxList();

    /**
     * @brief 设置数据
     */
    void SetData(int v);
    /**
     * @brief 获取数据
     */
    int GetData();
    /**
     * @brief 设置最大索引级别
     */
    void SetLevel(int l);

private:
    /**当前节点的值*/
    int m_data;
    /**
     * 当前节点的每个等级的下一个节点.
     * 第2级 N1 N2
     * 第1级 N1 N2
     * 如果N1是本节点,则 m_lpForwards[x] 保存的是N2
     *
     * [0] 就是原始链表.
     */
    CNode *m_lpForwards[MAX_LEVEL];
    /**当前节点的所在的最大索引级别*/
    int m_iMaxLevel;
};

/**
 * @brief 跳表
 */
class CSkipList {
public:
    CSkipList();
    ~CSkipList();
    /**
     * @brief 查找指定的值的节点
     * @param v 正整数
     */
    CNode *Find(int v);
    /**
     * @brief 插入指定的值
     * @param v 正整数
     */
    void Insert(int v);
    /**
     * @brief 删除指定的值的节点
     * @param v 正整数
     */
    int Delete(int v);
    void PrintAll();
    /**
     * @brief 打印跳表结构
     * @param l 等于-1时打印所有级别的结构 >=0时打印指定级别的结构
     */
    void PrintAll(int l);
    /**
     * @brief 插入节点时,得到插入K级的随机函数
     * @return K
     */
    int RandomLevel();

private:
    int levelCount;
    /**
     * 链表
     * 带头节点
     */
    CNode *m_lpHead;
};

CNode::CNode() {
    m_data = -1;
    m_iMaxLevel = 0;
    for (int i = 0; i < MAX_LEVEL; i++) {
        m_lpForwards[i] = NULL;
    }
}
CNode::~CNode() {}
CNode **CNode::GetIdxList() {
    return m_lpForwards;
}

void CNode::SetData(int v) {
    m_data = v;
}
int CNode::GetData() {
    return m_data;
}
void CNode::SetLevel(int l) {
    m_iMaxLevel = l;
}
std::string CNode::toString() {
    char tmp[32];
    std::string ret;

    ret.append("{ data: ");
    sprintf(tmp, "%d", m_data);
    ret.append(tmp);
    ret.append("; levels: ");
    sprintf(tmp, "%d", m_iMaxLevel);
    ret.append(tmp);
    ret.append(" }");
    return ret;
}

/**
 * 跳表相关的操作
 *
 */
CSkipList::CSkipList() {
    levelCount = 1;
    m_lpHead = new CNode();
}

CSkipList::~CSkipList() {}

CNode *CSkipList::Find(int v) {
    CNode *lpNode = m_lpHead;
    /**
     * 从 最大级索引链表开始查找.
     * K -> k-1 -> k-2 ...->0
     */
    for (int i = levelCount - 1; i >= 0; --i) {
        /**
         * 查找小于v的节点(lpNode).
         */
        while ((NULL != lpNode->GetIdxList()[i]) && (lpNode->GetIdxList()[i]->GetData() < v)) {
            lpNode = lpNode->GetIdxList()[i];
        }
    }
    /**
     * lpNode 是小于v的节点, lpNode的下一个节点就等于或大于v的节点
     */
    if ((NULL != lpNode->GetIdxList()[0]) && (lpNode->GetIdxList()[0]->GetData() == v)) {
        return lpNode->GetIdxList()[0];
    }
    return NULL;
}

void CSkipList::Insert(int v) {
    /// 新节点
    CNode *lpNewNode = new CNode();
    if (NULL == lpNewNode) {
        return;
    }

    /**
     * 新节点最大分布在的索引链表的上限
     * 如果返回 3,则 新的节点会在索引1、2、3上的链表都存在
     */
    int level = RandomLevel();
    lpNewNode->SetData(v);
    lpNewNode->SetLevel(level);

    /**
     * 临时索引链表
     * 主要是得到新的节点在每个索引链表上的位置
     */
    CNode *lpUpdateNode[level];
    for (int i = 0; i < level; i++) {
        /// 每个索引链表的头节点
        lpUpdateNode[i] = m_lpHead;
    }
    CNode *lpFind = m_lpHead;
    for (int i = level - 1; i >= 0; --i) {
        /**
         * 查找位置
         *   eg.  第1级  1  7  10
         *   如果插入的是 6
         *   lpFind->GetIdxList()[i]->GetData() : 表示节点lpFind在第1级索引的下一个节点的数据
         *   当 "lpFind->GetIdxList()[i]->GetData() < v"不成立的时候,
         *   新节点就要插入到 lpFind节点的后面, lpFind->GetIdxList()[i] 节点的前面
         *   即在这里 lpFind就是1  lpFind->GetIdxList()[i] 就是7
         */
        while ((NULL != lpFind->GetIdxList()[i]) && (lpFind->GetIdxList()[i]->GetData() < v)) {
            lpFind = lpFind->GetIdxList()[i];
        }
        /// lpFind 是新节点在 第i级索引链表的后一个节点
        lpUpdateNode[i] = lpFind;
    }

    for (int i = 0; i < level; ++i) {
        /**
         * 重新设置链表指针位置
         *   eg  第1级索引 1  7  10
         *      插入6.
         *      lpUpdateNode[i] 节点是1; lpUpdateNode[i]->GetIdxList()[i]节点是7
         *
         *  这2句代码就是 把6放在 1和7之间
         */
        lpNewNode->GetIdxList()[i] = lpUpdateNode[i]->GetIdxList()[i];
        lpUpdateNode[i]->GetIdxList()[i] = lpNewNode;
    }
    if (levelCount < level) {
        levelCount = level;
    }
}
int CSkipList::Delete(int v) {
    int ret = -1;
    CNode *lpUpdateNode[levelCount];
    CNode *lpFind = m_lpHead;
    for (int i = levelCount - 1; i >= 0; --i) {
        /**
         * 查找小于v的节点(lpFind).
         */
        while ((NULL != lpFind->GetIdxList()[i]) && (lpFind->GetIdxList()[i]->GetData() < v)) {
            lpFind = lpFind->GetIdxList()[i];
        }
        lpUpdateNode[i] = lpFind;
    }
    /**
     * lpFind 是小于v的节点, lpFind的下一个节点就等于或大于v的节点
     */
    if ((NULL != lpFind->GetIdxList()[0]) && (lpFind->GetIdxList()[0]->GetData() == v)) {
        for (int i = levelCount - 1; i >= 0; --i) {
            if ((NULL != lpUpdateNode[i]->GetIdxList()[i]) && (v == lpUpdateNode[i]->GetIdxList()[i]->GetData())) {
                lpUpdateNode[i]->GetIdxList()[i] = lpUpdateNode[i]->GetIdxList()[i]->GetIdxList()[i];
                ret = 0;
            }
        }
    }
    return ret;
}
void CSkipList::PrintAll() {
    CNode *lpNode = m_lpHead;
    while (NULL != lpNode->GetIdxList()[0]) {
        std::cout << lpNode->GetIdxList()[0]->toString().data() << std::endl;
        lpNode = lpNode->GetIdxList()[0];
    }
}
void CSkipList::PrintAll(int l) {
    for (int i = MAX_LEVEL - 1; i >= 0; --i) {
        CNode *lpNode = m_lpHead;
        std::cout << "第" << i << "级:" << std::endl;
        if ((l < 0) || ((l >= 0) && (l == i))) {
            while (NULL != lpNode->GetIdxList()[i]) {
                std::cout << lpNode->GetIdxList()[i]->GetData() << " ";
                lpNode = lpNode->GetIdxList()[i];
            }
            std::cout << std::endl;
            if (l >= 0) {
                break;
            }
        }
    }
}

// ! 生成随机数 1～99999
int GetRandom() {
    static std::default_random_engine generator(time(0));                   // 产生随机数的引擎
    std::uniform_int_distribution<int> distribution(1, 100 /*0x7FFFFFFF*/); // 分布器：均匀分布
    int dice_roll = distribution(generator);
    return dice_roll;
}

// 实际索引级别为 返回值减1，1表示不建立索引
int CSkipList::RandomLevel() {
    int level = 1; // 1 ~ MAX_LEVEL
    for (int i = 1; i < MAX_LEVEL; ++i) {
        if (1 == (rand() % 3)) {
            level++;
        }
        else
            break;
    }
    return level;
}