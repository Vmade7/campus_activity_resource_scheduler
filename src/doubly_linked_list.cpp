#ifndef DOUBLY_LINKED_LIST_CPP
#define DOUBLY_LINKED_LIST_CPP

// Node构造函数实现
template<typename T>
DoublyLinkedList<T>::Node:: Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}

// DoublyLinkedList构造函数
template<typename T>
DoublyLinkedList<T>:: DoublyLinkedList(bool check_duplicates) 
    : head(nullptr), tail(nullptr), size(0), enable_duplicate_check(check_duplicates) {}

// 析构函数
template<typename T>
DoublyLinkedList<T>::~DoublyLinkedList() {
    clear();
}

// 内部查重函数
template<typename T>
bool DoublyLinkedList<T>::containsValue(const T& value) const {
    if (! enable_duplicate_check) return false;
    return find(value);
}

// 头部插入（带去重检查）
template<typename T>
void DoublyLinkedList<T>::pushFront(const T& value) {
    if (enable_duplicate_check && containsValue(value)) {
        std::cout << "重复检测:  值 " << value << " 已存在，跳过插入\n";
        return;  // 重复，不插入
    }
    
    Node* newNode = new Node(value);
    
    if (empty()) {
        head = tail = newNode;
    } else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    size++;
}

// 尾部插入（带去重检查）
template<typename T>
void DoublyLinkedList<T>:: pushBack(const T& value) {
    if (enable_duplicate_check && containsValue(value)) {
        std:: cout << "重复检测: 值 " << value << " 已存在，跳过插入\n";
        return;  // 重复，不插入
    }
    
    Node* newNode = new Node(value);
    
    if (empty()) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    size++;
}

// 头部删除
template<typename T>
void DoublyLinkedList<T>:: popFront() {
    if (empty()) return;
    
    Node* oldHead = head;
    
    if (size == 1) {
        head = tail = nullptr;
    } else {
        head = head->next;
        head->prev = nullptr;
    }
    
    delete oldHead;
    size--;
}

// 尾部删除
template<typename T>
void DoublyLinkedList<T>::popBack() {
    if (empty()) return;
    
    Node* oldTail = tail;
    
    if (size == 1) {
        head = tail = nullptr;
    } else {
        tail = tail->prev;
        tail->next = nullptr;
    }
    
    delete oldTail;
    size--;
}

// 查找元素
template<typename T>
bool DoublyLinkedList<T>:: find(const T& value) const {
    Node* current = head;
    while (current != nullptr) {
        if (current->data == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// 获取头部元素（非const版本）
template<typename T>
T& DoublyLinkedList<T>::front() {
    if (empty()) throw std::runtime_error("List is empty");
    return head->data;
}

// 获取尾部元素（非const版本）
template<typename T>
T& DoublyLinkedList<T>::back() {
    if (empty()) throw std::runtime_error("List is empty");
    return tail->data;
}

// 获取头部元素（const版本）
template<typename T>
const T& DoublyLinkedList<T>::front() const {
    if (empty()) throw std::runtime_error("List is empty");
    return head->data;
}

// 获取尾部元素（const版本）
template<typename T>
const T& DoublyLinkedList<T>::back() const {
    if (empty()) throw std::runtime_error("List is empty");
    return tail->data;
}

// 指定位置插入（带去重检查）
template<typename T>
void DoublyLinkedList<T>::insert(size_t index, const T& value) {
    if (index > size) throw std::out_of_range("Index out of range");
    
    if (enable_duplicate_check && containsValue(value)) {
        std::cout << "重复检测: 值 " << value << " 已存在，跳过插入\n";
        return;  // 重复，不插入
    }
    
    if (index == 0) {
        pushFront(value);
        return;
    }
    
    if (index == size) {
        pushBack(value);
        return;
    }
    
    Node* newNode = new Node(value);
    Node* current = head;
    
    // 找到插入位置
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    // 插入新节点
    newNode->next = current;
    newNode->prev = current->prev;
    current->prev->next = newNode;
    current->prev = newNode;
    
    size++;
}

// 指定位置删除
template<typename T>
void DoublyLinkedList<T>::remove(size_t index) {
    if (index >= size) throw std::out_of_range("Index out of range");
    
    if (index == 0) {
        popFront();
        return;
    }
    
    if (index == size - 1) {
        popBack();
        return;
    }
    
    Node* current = head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    // 删除节点
    current->prev->next = current->next;
    current->next->prev = current->prev;
    delete current;
    size--;
}

// 删除指定值
template<typename T>
void DoublyLinkedList<T>::removeValue(const T& value) {
    Node* current = head;
    
    while (current != nullptr) {
        if (current->data == value) {
            if (current == head) {
                popFront();
            } else if (current == tail) {
                popBack();
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                delete current;
                size--;
            }
            return;
        }
        current = current->next;
    }
}

// 直接删除节点（内部使用）
template<typename T>
void DoublyLinkedList<T>::removeNodeDirectly(Node* node) {
    if (! node) return;
    
    if (node == head && node == tail) {
        // 只有一个节点
        head = tail = nullptr;
    } else if (node == head) {
        // 删除头节点
        head = head->next;
        head->prev = nullptr;
    } else if (node == tail) {
        // 删除尾节点
        tail = tail->prev;
        tail->next = nullptr;
    } else {
        // 删除中间节点
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    
    delete node;
    size--;
}

// 启用/禁用重复检查
template<typename T>
void DoublyLinkedList<T>::enableDuplicateCheck(bool enable) {
    enable_duplicate_check = enable;
    if (enable) {
        std::cout << "去重检查已启用\n";
    } else {
        std::cout << "去重检查已禁用\n";
    }
}

// 检查去重是否启用
template<typename T>
bool DoublyLinkedList<T>::isDuplicateCheckEnabled() const {
    return enable_duplicate_check;
}

// 去除重复项
template<typename T>
size_t DoublyLinkedList<T>::removeDuplicates() {
    if (empty()) return 0;
    
    size_t removed_count = 0;
    Node* current = head;
    
    while (current != nullptr) {
        Node* runner = current->next;
        
        // 查找并删除后续的重复节点
        while (runner != nullptr) {
            if (runner->data == current->data) {
                Node* duplicate = runner;
                runner = runner->next;
                
                // 删除重复节点
                removeNodeDirectly(duplicate);
                removed_count++;
            } else {
                runner = runner->next;
            }
        }
        
        current = current->next;
    }
    
    return removed_count;
}

// 查找重复项
template<typename T>
std::vector<T> DoublyLinkedList<T>::findDuplicates() const {
    std::vector<T> duplicates;
    if (empty()) return duplicates;
    
    Node* current = head;
    while (current != nullptr) {
        Node* runner = current->next;
        bool found_duplicate = false;
        
        while (runner != nullptr) {
            if (runner->data == current->data && !found_duplicate) {
                duplicates.push_back(current->data);
                found_duplicate = true;
                break; // 找到一个重复就够了，避免重复添加
            }
            runner = runner->next;
        }
        
        current = current->next;
    }
    
    return duplicates;
}

// 统计重复项数量
template<typename T>
size_t DoublyLinkedList<T>:: countDuplicates() const {
    if (empty()) return 0;
    
    size_t duplicate_count = 0;
    Node* current = head;
    
    while (current != nullptr) {
        Node* runner = current->next;
        
        while (runner != nullptr) {
            if (runner->data == current->data) {
                duplicate_count++;
            }
            runner = runner->next;
        }
        
        current = current->next;
    }
    
    return duplicate_count;
}

// 检查是否为空
template<typename T>
bool DoublyLinkedList<T>::empty() const {
    return size == 0;
}

// 获取大小
template<typename T>
size_t DoublyLinkedList<T>::getSize() const {
    return size;
}

// 清空链表
template<typename T>
void DoublyLinkedList<T>:: clear() {
    while (!empty()) {
        popFront();
    }
}

// 反转链表
template<typename T>
void DoublyLinkedList<T>:: reverse() {
    if (size <= 1) return;
    
    Node* current = head;
    
    // 交换每个节点的prev和next指针
    while (current != nullptr) {
        Node* temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev; // 注意：因为prev和next已经交换了
    }
    
    // 交换头尾指针
    Node* temp = head;
    head = tail;
    tail = temp;
}

// 遍历修改版本
template<typename T>
void DoublyLinkedList<T>::forEach(std::function<void(T&)> func) {
    Node* current = head;
    while (current != nullptr) {
        func(current->data);
        current = current->next;
    }
}

// 遍历只读版本
template<typename T>
void DoublyLinkedList<T>::forEach(std::function<void(const T&)> func) const {
    Node* current = head;
    while (current != nullptr) {
        func(current->data);
        current = current->next;
    }
}

// 打印链表
template<typename T>
void DoublyLinkedList<T>:: print() const {
    std::cout << "链表内容 (size=" << size << "): ";
    
    if (empty()) {
        std::cout << "[空]";
    } else {
        std::cout << "[";
        Node* current = head;
        while (current != nullptr) {
            std::cout << current->data;
            if (current->next != nullptr) {
                std::cout << " ←→ ";
            }
            current = current->next;
        }
        std:: cout << "]";
    }
    std::cout << std::endl;
}

// 打印链表及重复信息
template<typename T>
void DoublyLinkedList<T>::printWithDuplicateInfo() const {
    print();
    
    std::cout << "去重状态:  " << (enable_duplicate_check ? "已启用" :  "已禁用") << std::endl;
    
    if (! empty()) {
        size_t dup_count = countDuplicates();
        std::cout << "重复项数量: " << dup_count << std::endl;
        
        if (dup_count > 0) {
            auto duplicates = findDuplicates();
            std::cout << "重复的值: ";
            for (const auto& dup : duplicates) {
                std:: cout << dup << " ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std:: endl;
}

// Iterator构造函数
template<typename T>
DoublyLinkedList<T>::Iterator:: Iterator(Node* node) : current(node) {}

// Iterator解引用操作符
template<typename T>
T& DoublyLinkedList<T>::Iterator:: operator*() {
    return current->data;
}

// Iterator前置递增操作符
template<typename T>
typename DoublyLinkedList<T>::Iterator& DoublyLinkedList<T>::Iterator::operator++() {
    current = current->next;
    return *this;
}

// Iterator前置递减操作符
template<typename T>
typename DoublyLinkedList<T>:: Iterator& DoublyLinkedList<T>::Iterator::operator--() {
    current = current->prev;
    return *this;
}

// Iterator不等于操作符
template<typename T>
bool DoublyLinkedList<T>::Iterator::operator!=(const Iterator& other) const {
    return current != other. current;
}

// Iterator等于操作符
template<typename T>
bool DoublyLinkedList<T>::Iterator::operator==(const Iterator& other) const {
    return current == other.current;
}

// begin迭代器
template<typename T>
typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::begin() {
    return Iterator(head);
}

// end迭代器
template<typename T>
typename DoublyLinkedList<T>::Iterator DoublyLinkedList<T>::end() {
    return Iterator(nullptr);
}

#endif // DOUBLY_LINKED_LIST_CPP