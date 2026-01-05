#include "../include/hash_table.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>

// 静态常量定义
const double HashTable::MAX_LOAD_FACTOR = 0.75;
const size_t HashTable::MIN_CAPACITY = 16;

HashTable::HashTable(size_t initial_capacity) 
    : table_size(std::max(initial_capacity, MIN_CAPACITY)), element_count(0) {
    table.resize(table_size, nullptr);
}

HashTable::~HashTable() {
    clear();
}

size_t HashTable::hashFunction(int key) const {
    // 使用简单但有效的哈希函数
    return static_cast<size_t>(key * 2654435761U) % table_size;
}

size_t HashTable::getIndex(int key) const {
    return hashFunction(key);
}

bool HashTable::insert(std::shared_ptr<Contact> contact) {
    if (! contact) return false;
    
    // 检查负载因子是否需要扩容
    if (loadFactor() > MAX_LOAD_FACTOR) {
        resize();
    }
    
    size_t index = getIndex(contact->id);
    
    // 检查是否已存在
    HashNode* current = table[index];
    while (current != nullptr) {
        if (current->contact->id == contact->id) {
            // ID已存在，更新联系人信息
            current->contact = contact;
            return true;
        }
        current = current->next;
    }
    
    // 不存在，插入新节点（头插法）
    HashNode* newNode = new HashNode(contact);
    newNode->next = table[index];
    table[index] = newNode;
    element_count++;
    
    return true;
}

std::shared_ptr<Contact> HashTable::find(int key) {
    HashNode* node = findNode(key);
    return node ? node->contact : nullptr;
}

std::shared_ptr<Contact> HashTable::findByStudentId(const std::string& student_id) {
    // 遍历所有桶查找学号（效率较低，但功能完整）
    for (size_t i = 0; i < table_size; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            if (current->contact->student_id == student_id) {
                return current->contact;
            }
            current = current->next;
        }
    }
    return nullptr;
}

HashTable::HashNode* HashTable:: findNode(int key) {
    size_t index = getIndex(key);
    HashNode* current = table[index];
    
    while (current != nullptr) {
        if (current->contact->id == key) {
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

bool HashTable::remove(int key) {
    size_t index = getIndex(key);
    HashNode* current = table[index];
    HashNode* prev = nullptr;
    
    while (current != nullptr) {
        if (current->contact->id == key) {
            // 找到要删除的节点
            if (prev == nullptr) {
                // 删除头节点
                table[index] = current->next;
            } else {
                // 删除中间或尾节点
                prev->next = current->next;
            }
            delete current;
            element_count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false; // 未找到
}

bool HashTable::contains(int key) {
    return findNode(key) != nullptr;
}

bool HashTable::isDuplicate(const Contact& contact) {
    // 检查ID是否重复
    if (contains(contact.id)) {
        return true;
    }
    
    // 检查学号是否重复
    if (findByStudentId(contact. student_id) != nullptr) {
        return true;
    }
    
    return false;
}

std:: vector<std::shared_ptr<Contact>> HashTable::findDuplicates() {
    std::vector<std::shared_ptr<Contact>> duplicates;
    std::unordered_set<std::string> seen_student_ids;
    std::unordered_set<int> seen_ids;
    
    for (size_t i = 0; i < table_size; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            bool is_duplicate = false;
            
            // 检查ID重复
            if (seen_ids.count(current->contact->id) > 0) {
                is_duplicate = true;
            } else {
                seen_ids.insert(current->contact->id);
            }
            
            // 检查学号重复
            if (seen_student_ids.count(current->contact->student_id) > 0) {
                is_duplicate = true;
            } else {
                seen_student_ids.insert(current->contact->student_id);
            }
            
            if (is_duplicate) {
                duplicates. push_back(current->contact);
            }
            
            current = current->next;
        }
    }
    
    return duplicates;
}

size_t HashTable::removeDuplicates() {
    auto duplicates = findDuplicates();
    size_t removed_count = 0;
    
    for (const auto& duplicate :  duplicates) {
        if (remove(duplicate->id)) {
            removed_count++;
        }
    }
    
    return removed_count;
}

void HashTable::resize() {
    size_t old_size = table_size;
    std::vector<HashNode*> old_table = std::move(table);
    
    // 扩容为原来的2倍
    table_size = old_size * 2;
    table.assign(table_size, nullptr);
    element_count = 0; // 重新计数
    
    // 重新插入所有元素
    for (size_t i = 0; i < old_size; i++) {
        HashNode* current = old_table[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            
            // 重新计算哈希值并插入
            insertHelper(current->contact, table);
            delete current; // 删除旧节点
            
            current = next;
        }
    }
}

void HashTable::insertHelper(std::shared_ptr<Contact> contact, 
                           std::vector<HashNode*>& target_table) {
    size_t index = getIndex(contact->id);
    
    HashNode* newNode = new HashNode(contact);
    newNode->next = target_table[index];
    target_table[index] = newNode;
    element_count++;
}

size_t HashTable::size() const {
    return element_count;
}

size_t HashTable::capacity() const {
    return table_size;
}

double HashTable::loadFactor() const {
    return table_size > 0 ? static_cast<double>(element_count) / table_size : 0.0;
}

void HashTable::clear() {
    for (size_t i = 0; i < table_size; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            HashNode* next = current->next;
            delete current;
            current = next;
        }
        table[i] = nullptr;
    }
    element_count = 0;
}

std::vector<std::shared_ptr<Contact>> HashTable::getAllContacts() {
    std::vector<std::shared_ptr<Contact>> all_contacts;
    all_contacts.reserve(element_count);
    
    for (size_t i = 0; i < table_size; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            all_contacts.push_back(current->contact);
            current = current->next;
        }
    }
    
    return all_contacts;
}

size_t HashTable::getChainLength(size_t index) const {
    if (index >= table_size) return 0;
    
    size_t length = 0;
    HashNode* current = table[index];
    while (current != nullptr) {
        length++;
        current = current->next;
    }
    return length;
}

size_t HashTable::getMaxChainLength() const {
    size_t max_length = 0;
    for (size_t i = 0; i < table_size; i++) {
        max_length = std::max(max_length, getChainLength(i));
    }
    return max_length;
}

double HashTable::getAverageChainLength() const {
    if (element_count == 0) return 0.0;
    
    size_t non_empty_buckets = 0;
    for (size_t i = 0; i < table_size; i++) {
        if (table[i] != nullptr) {
            non_empty_buckets++;
        }
    }
    
    return non_empty_buckets > 0 ? 
           static_cast<double>(element_count) / non_empty_buckets : 0.0;
}

void HashTable:: printStatistics() const {
    std::cout << "=== 哈希表统计信息 ===\n";
    std:: cout << "容量: " << capacity() << "\n";
    std::cout << "元素数量: " << size() << "\n";
    std::cout << "负载因子: " << loadFactor() << "\n";
    std::cout << "最大链长度:  " << getMaxChainLength() << "\n";
    std:: cout << "平均链长度: " << getAverageChainLength() << "\n";
    
    // 计算空桶数量
    size_t empty_buckets = 0;
    for (size_t i = 0; i < table_size; i++) {
        if (table[i] == nullptr) {
            empty_buckets++;
        }
    }
    std::cout << "空桶数量:  " << empty_buckets << " (" 
              << (static_cast<double>(empty_buckets) / table_size * 100) << "%)\n";
    std::cout << "========================\n\n";
}

void HashTable::printDistribution() const {
    std::cout << "=== 哈希表分布情况 ===\n";
    
    for (size_t i = 0; i < std::min(table_size, size_t(20)); i++) { // 只显示前20个桶
        size_t chain_length = getChainLength(i);
        std::cout << "桶 " << i << ": ";
        
        if (chain_length == 0) {
            std:: cout << "[空]";
        } else {
            std::cout << "[" << chain_length << " 个元素] ";
            HashNode* current = table[i];
            int count = 0;
            while (current != nullptr && count < 3) { // 只显示前3个元素
                std::cout << current->contact->name << "(" << current->contact->id << ") ";
                current = current->next;
                count++;
            }
            if (chain_length > 3) {
                std::cout << "...  ";
            }
        }
        std::cout << "\n";
    }
    
    if (table_size > 20) {
        std::cout << "... (还有 " << (table_size - 20) << " 个桶)\n";
    }
    std::cout << "====================\n\n";
}