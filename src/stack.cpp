#ifndef STACK_CPP
#define STACK_CPP

// 包含头文件
#include "../include/stack.h"

// Node构造函数
template<typename T>
Stack<T>::Node::Node(const T& value, Node* nextNode) : data(value), next(nextNode) {}

// Stack构造函数
template<typename T>
Stack<T>::Stack() : topNode(nullptr), stackSize(0) {}

// Stack析构函数
template<typename T>
Stack<T>::~Stack() {
    clear();
}

// 入栈
template<typename T>
void Stack<T>::push(const T& value) {
    Node* newNode = new Node(value, topNode);  // 创建新节点
    topNode = newNode;  // 更新栈顶
    stackSize++;
}

// 出栈
template<typename T>
void Stack<T>::pop() {
    if (empty()) {
        throw std::underflow_error("Stack is empty. Cannot pop.");
    }
    
    Node* oldTop = topNode;
    topNode = topNode->next;  // 更新栈顶
    delete oldTop;            // 删除旧栈顶
    stackSize--;
}

// 获取栈顶元素（非const）
template<typename T>
T& Stack<T>::top() {
    if (empty()) {
        throw std::underflow_error("Stack is empty. No top element.");
    }
    return topNode->data;
}

// 获取栈顶元素（const）
template<typename T>
const T& Stack<T>::top() const {
    if (empty()) {
        throw std::underflow_error("Stack is empty. No top element.");
    }
    return topNode->data;
}

// 检查栈是否为空
template<typename T>
bool Stack<T>::empty() const {
    return stackSize == 0;
}

// 获取栈的大小
template<typename T>
size_t Stack<T>::size() const {
    return stackSize;
}

// 清空栈
template<typename T>
void Stack<T>::clear() {
    while (!empty()) {
        pop();
    }
}

// 打印整个栈（从栈顶到栈底）
template<typename T>
void Stack<T>::print() const {
    std::cout << "栈内容（从栈顶到栈底）: ";
    Node* current = topNode;
    while (current != nullptr) {
        std::cout << current->data << " ";
        current = current->next;
    }
    std::cout << "\n";
}

#endif // STACK_CPP