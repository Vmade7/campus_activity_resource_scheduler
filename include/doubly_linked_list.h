#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <iostream>
#include <functional>
#include <vector>

template<typename T>
class DoublyLinkedList {
private:  
    struct Node {
        T data;
        Node* prev;
        Node* next;
        
        Node(const T& value);
    };
    
    Node* head;                     // 头指针
    Node* tail;                     // 尾指针  
    size_t size;                    // 链表大小
    bool enable_duplicate_check;    // 是否启用重复检查

public:
    // 构造和析构
    DoublyLinkedList(bool check_duplicates = false);
    ~DoublyLinkedList();
    
    // 基本操作
    void pushFront(const T& value);      // 头部插入
    void pushBack(const T& value);       // 尾部插入
    void popFront();                     // 头部删除
    void popBack();                      // 尾部删除
    
    // 查找和访问
    bool find(const T& value) const;     // 查找元素
    T& front();                          // 获取头部元素
    T& back();                           // 获取尾部元素
    const T& front() const;
    const T& back() const;
    
    // 位置操作
    void insert(size_t index, const T& value);  // 指定位置插入
    void remove(size_t index);                  // 指定位置删除
    void removeValue(const T& value);           // 删除指定值
    
    // 去重功能
    void enableDuplicateCheck(bool enable = true);   // 启用/禁用重复检查
    bool isDuplicateCheckEnabled() const;            // 检查去重是否启用
    size_t removeDuplicates();                       // 移除所有重复项
    std::vector<T> findDuplicates() const;           // 查找重复项
    size_t countDuplicates() const;                  // 统计重复项数量
    
    // 工具函数
    bool empty() const;                  // 检查是否为空
    size_t getSize() const;              // 获取大小
    void clear();                        // 清空链表
    void reverse();                      // 反转链表
    
    // 遍历功能
    void forEach(std::function<void(T&)> func);           // 遍历修改
    void forEach(std::function<void(const T&)> func) const; // 遍历只读
    void print() const;                                   // 打印链表
    void printWithDuplicateInfo() const;                  // 打印链表及重复信息
    
    class Iterator {
    private:  
        Node* current;
    public:  
        Iterator(Node* node);
        
        T& operator*();
        Iterator& operator++();
        Iterator& operator--();
        bool operator!=(const Iterator& other) const;
        bool operator==(const Iterator& other) const;
    };
    
    Iterator begin();
    Iterator end();

private:
    bool containsValue(const T& value) const;     // 内部查重函数
    void removeNodeDirectly(Node* node);          // 直接删除节点（内部使用）
};

#include "../src/doubly_linked_list.cpp"
#endif // DOUBLY_LINKED_LIST_H