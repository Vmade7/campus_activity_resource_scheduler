#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <algorithm>

class PriorityQueue {
public:
    PriorityQueue();
    void push(int value);  // 插入操作
    int pop();  // 弹出最小值
    bool isEmpty();  // 判断队列是否为空

private:
    std::vector<int> heap;
    void heapifyUp(int index);
    void heapifyDown(int index);
};

#endif // PRIORITY_QUEUE_H