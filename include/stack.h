#ifndef STACK_H
#define STACK_H

#include <iostream>
#include <stdexcept>

template<typename T>
class Stack {
private:
    struct Node {
        T data;       // 数据
        Node* next;   // 指向下一个节点
        
        Node(const T& value, Node* nextNode = nullptr);
    };
    
    Node* topNode;     // 栈顶指针
    size_t stackSize;  // 栈中的元素数量

public:
    // 构造和析构
    Stack();                                // 默认构造函数
    ~Stack();                               // 析构函数
    Stack(const Stack& other);              // 拷贝构造函数（实现深拷贝）
    Stack& operator=(const Stack& other);   // 拷贝赋值运算符
    Stack(Stack&& other) noexcept;          // 移动构造函数
    Stack& operator=(Stack&& other) noexcept; // 移动赋值运算符
    
    // 禁用拷贝和赋值（仅适用于禁用方案）
    // Stack(const Stack& other) = delete;
    // Stack& operator=(const Stack& other) = delete;

    // 栈的基本操作
    void push(const T& value);      // 入栈
    void pop();                     // 出栈
    T& top();                       // 获取栈顶元素（非const）
    const T& top() const;           // 获取栈顶元素（const）
    
    // 工具函数
    bool empty() const;             // 检查栈是否为空
    size_t size() const;            // 获取栈的大小
    void clear();                   // 清空栈
    
    // 调试扩展
    void print() const;             // 打印整个栈（从栈顶到栈底）
};

// Node构造函数实现
template<typename T>
Stack<T>::Node::Node(const T& value, Node* nextNode) : data(value), next(nextNode) {}

// 默认构造函数
template<typename T>
Stack<T>::Stack() : topNode(nullptr), stackSize(0) {}

// 析构函数
template<typename T>
Stack<T>::~Stack() {
    clear();
}

// 拷贝构造函数（深拷贝）
template<typename T>
Stack<T>::Stack(const Stack& other) : topNode(nullptr), stackSize(0) {
    if (!other.empty()) {
        Node* current = other.topNode;
        Stack<T> tempStack;  // 创建临时栈用于反转
        
        // 逆序遍历原栈
        while (current != nullptr) {
            tempStack.push(current->data);
            current = current->next;
        }
        
        // 依次将临时栈的元素推入当前栈
        while (!tempStack.empty()) {
            this->push(tempStack.top());
            tempStack.pop();
        }
    }
}

// 拷贝赋值运算符（深拷贝）
template<typename T>
Stack<T>& Stack<T>::operator=(const Stack& other) {
    if (this != &other) {
        clear();  // 清空当前栈
        
        if (!other.empty()) {
            Node* current = other.topNode;
            Stack<T> tempStack;  // 创建临时栈用于反转
            
            // 逆序遍历原栈
            while (current != nullptr) {
                tempStack.push(current->data);
                current = current->next;
            }
            
            // 依次将临时栈的元素推入当前栈
            while (!tempStack.empty()) {
                this->push(tempStack.top());
                tempStack.pop();
            }
        }
    }
    return *this;
}

// 移动构造函数
template<typename T>
Stack<T>::Stack(Stack&& other) noexcept : topNode(other.topNode), stackSize(other.stackSize) {
    other.topNode = nullptr;
    other.stackSize = 0;
}

// 移动赋值运算符
template<typename T>
Stack<T>& Stack<T>::operator=(Stack&& other) noexcept {
    if (this != &other) {
        clear();
        topNode = other.topNode;
        stackSize = other.stackSize;
        other.topNode = nullptr;
        other.stackSize = 0;
    }
    return *this;
}

// 入栈
template<typename T>
void Stack<T>::push(const T& value) {
    Node* newNode = new Node(value, topNode);
    topNode = newNode;
    stackSize++;
}

// 出栈
template<typename T>
void Stack<T>::pop() {
    if (empty()) {
        throw std::underflow_error("Stack is empty. Cannot pop.");
    }
    Node* oldTop = topNode;
    topNode = topNode->next;
    delete oldTop;
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

// 打印整个栈（加上空栈提示）
template<typename T>
void Stack<T>::print()const {
    if (empty()) {
        std::cout << "栈内容（从栈顶到栈底）: 空栈\n";
        return;
    }
    std::cout << "栈内容（从栈顶到栈底）: ";
    Node* current = topNode;
    while (current != nullptr) {
        std::cout << current->data << " ";
        current = current->next;
    }
    std::cout << "\n";
}

#endif // STACK_H