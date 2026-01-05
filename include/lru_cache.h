#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <iostream>

template<typename K, typename V>
class LRUCache {
private:
    // 双向链表节点
    struct Node {
        K key;
        V value;
        Node* prev;
        Node* next;
        
        Node() : prev(nullptr), next(nullptr) {}
        Node(K k, V v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };
    
    std::unordered_map<K, Node*> cache;  // 哈希表：key -> 节点指针
    Node* head;                          // 虚拟头节点（最新）
    Node* tail;                          // 虚拟尾节点（最旧）
    size_t capacity;                     // 缓存容量
    size_t currentSize;                  // 当前大小
    
    // 内部辅助函数
    void addToHead(Node* node);          // 添加节点到头部
    void removeNode(Node* node);         // 从链表中移除节点
    void moveToHead(Node* node);         // 移动节点到头部  
    Node* removeTail();                  // 移除尾部节点

public:
    LRUCache(size_t cap);
    ~LRUCache();
    
    // 主要接口
    V get(const K& key);                 // 获取值
    void put(const K& key, const V& value);  // 插入/更新值
    bool contains(const K& key) const;   // 检查key是否存在
    void remove(const K& key);           // 删除key
    
    // 工具函数
    size_t size() const;                 // 获取当前大小
    size_t getCapacity() const;          // 获取缓存容量
    bool empty() const;                  // 检查是否为空
    void clear();                        // 清空缓存
    void printCache() const;             // 打印缓存状态（调试用）
    
    // 统计信息
    void printStats() const;             // 打印统计信息
};

// 实现部分
template<typename K, typename V>
LRUCache<K, V>:: LRUCache(size_t cap) : capacity(cap), currentSize(0) {
    // 创建虚拟头尾节点
    head = new Node();
    tail = new Node();
    
    // 连接头尾节点
    head->next = tail;
    tail->prev = head;
}

template<typename K, typename V>
LRUCache<K, V>:: ~LRUCache() {
    clear();
    delete head;
    delete tail;
}

template<typename K, typename V>
void LRUCache<K, V>::addToHead(Node* node) {
    // 在head后面插入node
    node->prev = head;
    node->next = head->next;
    
    head->next->prev = node;
    head->next = node;
}

template<typename K, typename V>
void LRUCache<K, V>:: removeNode(Node* node) {
    // 从双向链表中移除node
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

template<typename K, typename V>
void LRUCache<K, V>::moveToHead(Node* node) {
    // 先从当前位置移除，再添加到头部
    removeNode(node);
    addToHead(node);
}

template<typename K, typename V>
typename LRUCache<K, V>::Node* LRUCache<K, V>:: removeTail() {
    // 移除尾部的前一个节点（最旧的数据）
    Node* lastNode = tail->prev;
    removeNode(lastNode);
    return lastNode;
}

template<typename K, typename V>
V LRUCache<K, V>::get(const K& key) {
    auto it = cache.find(key);
    if (it == cache.end()) {
        // key不存在，返回默认构造的V
        return V{};
    }
    
    // key存在，移动到头部（标记为最近使用）
    Node* node = it->second;
    moveToHead(node);
    
    return node->value;
}

template<typename K, typename V>
void LRUCache<K, V>::put(const K& key, const V& value) {
    auto it = cache.find(key);
    
    if (it != cache.end()) {
        // key已存在，更新value并移到头部
        Node* node = it->second;
        node->value = value;
        moveToHead(node);
        return;
    }
    
    // key不存在，创建新节点
    Node* newNode = new Node(key, value);
    
    if (currentSize >= capacity) {
        // 缓存已满，移除最旧的节点
        Node* tail_node = removeTail();
        cache.erase(tail_node->key);
        delete tail_node;
        currentSize--;
    }
    
    // 添加新节点到头部
    addToHead(newNode);
    cache[key] = newNode;
    currentSize++;
}

template<typename K, typename V>
bool LRUCache<K, V>::contains(const K& key) const {
    return cache.find(key) != cache.end();
}

template<typename K, typename V>
void LRUCache<K, V>::remove(const K& key) {
    auto it = cache.find(key);
    if (it != cache. end()) {
        Node* node = it->second;
        removeNode(node);
        cache.erase(it);
        delete node;
        currentSize--;
    }
}

template<typename K, typename V>
size_t LRUCache<K, V>::size() const {
    return currentSize;
}

template<typename K, typename V>
size_t LRUCache<K, V>::getCapacity() const {
    return capacity;
}

template<typename K, typename V>
bool LRUCache<K, V>::empty() const {
    return currentSize == 0;
}

template<typename K, typename V>
void LRUCache<K, V>::clear() {
    while (currentSize > 0) {
        Node* node = removeTail();
        cache.erase(node->key);
        delete node;
        currentSize--;
    }
}

template<typename K, typename V>
void LRUCache<K, V>::printCache() const {
    std::cout << "=== LRU Cache 状态 ===\n";
    std::cout << "容量: " << capacity << ", 当前大小: " << currentSize << "\n";
    std::cout << "从新到旧:  ";
    
    Node* current = head->next;
    while (current != tail) {
        std::cout << "[" << current->key << ":" << current->value << "] ";
        current = current->next;
    }
    std::cout << "\n\n";
}

#endif // LRU_CACHE_H