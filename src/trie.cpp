#include "trie.h"
#include <iostream>
#include <algorithm>
#include <set>

Trie::Trie() : contactCount(0) {
    root = new TrieNode();
}

Trie::~Trie() {
    deleteNode(root);
}

void Trie::deleteNode(TrieNode* node) {
    if (! node) return;
    
    for (auto& child : node->children) {
        deleteNode(child.second);
    }
    delete node;
}

// UTF-8字符处理函数
std::vector<std::string> Trie::splitUTF8Characters(const std::string& str) const {
    std::vector<std::string> characters;
    
    for (size_t i = 0; i < str. length(); ) {
        unsigned char byte = static_cast<unsigned char>(str[i]);
        int charLen = getUTF8CharLength(byte);
        
        // 防止越界
        if (i + charLen > str.length()) {
            charLen = str.length() - i;
        }
        
        characters.push_back(str.substr(i, charLen));
        i += charLen;
    }
    
    return characters;
}

bool Trie::isUTF8Start(unsigned char byte) const {
    return (byte & 0x80) == 0 || (byte & 0xC0) == 0xC0;
}

int Trie::getUTF8CharLength(unsigned char byte) const {
    if ((byte & 0x80) == 0) return 1;      // ASCII:  0xxxxxxx
    if ((byte & 0xE0) == 0xC0) return 2;   // 110xxxxx
    if ((byte & 0xF0) == 0xE0) return 3;   // 1110xxxx (中文字符)
    if ((byte & 0xF8) == 0xF0) return 4;   // 11110xxx
    return 1; // 默认返回1
}

// 联系人操作函数
void Trie::insertContact(std::shared_ptr<Contact> contact) {
    if (!contact) return;
    
    // 按姓名建索引
    auto nameChars = splitUTF8Characters(contact->name);
    TrieNode* current = root;
    
    for (const auto& ch : nameChars) {
        if (current->children.find(ch) == current->children.end()) {
            current->children[ch] = new TrieNode();
        }
        current = current->children[ch];
        current->contacts.push_back(contact);
    }
    current->isEndOfWord = true;
    
    // 按学号建索引
    auto idChars = splitUTF8Characters(contact->student_id);
    current = root;
    
    for (const auto& ch :  idChars) {
        if (current->children.find(ch) == current->children.end()) {
            current->children[ch] = new TrieNode();
        }
        current = current->children[ch];
        current->contacts.push_back(contact);
    }
    current->isEndOfWord = true;
    
    contactCount++;
}

std::vector<std::shared_ptr<Contact>> Trie::searchByNamePrefix(const std::string& prefix) {
    std::vector<std::shared_ptr<Contact>> result;
    
    auto chars = splitUTF8Characters(prefix);
    TrieNode* current = root;
    
    // 定位到前缀节点
    for (const auto& ch :  chars) {
        auto it = current->children.find(ch);
        if (it == current->children.end()) {
            return result; // 前缀不存在
        }
        current = it->second;
    }
    
    // 收集所有匹配的联系人
    collectAllContacts(current, result);
    
    // 过滤：只保留姓名确实以prefix开头的联系人
    std::vector<std:: shared_ptr<Contact>> filtered;
    for (const auto& contact :  result) {
        if (contact->name.find(prefix) == 0) {
            filtered.push_back(contact);
        }
    }
    
    return removeDuplicates(filtered);
}

std::vector<std::shared_ptr<Contact>> Trie::searchByStudentIdPrefix(const std::string& prefix) {
    std:: vector<std::shared_ptr<Contact>> result;
    
    auto chars = splitUTF8Characters(prefix);
    TrieNode* current = root;
    
    // 定位到前缀节点
    for (const auto& ch : chars) {
        auto it = current->children.find(ch);
        if (it == current->children.end()) {
            return result; // 前缀不存在
        }
        current = it->second;
    }
    
    // 收集所有匹配的联系人
    collectAllContacts(current, result);
    
    // 过滤：只保留学号确实以prefix开头的联系人
    std:: vector<std::shared_ptr<Contact>> filtered;
    for (const auto& contact : result) {
        if (contact->student_id.find(prefix) == 0) {
            filtered.push_back(contact);
        }
    }
    
    return removeDuplicates(filtered);
}

bool Trie:: deleteContact(int contactId) {
    // 标记删除（简化实现，实际项目中可能需要从树中完全移除）
    // 这里我们先实现一个基础版本
    // TODO: 完整的删除实现会比较复杂，需要重建受影响的路径
    return false; // 暂时返回false，表示功能待完善
}

// 辅助函数
void Trie:: collectAllContacts(TrieNode* node, std::vector<std::shared_ptr<Contact>>& result) const {
    if (! node) return;
    
    // 添加当前节点的所有联系人
    for (const auto& contact : node->contacts) {
        result.push_back(contact);
    }
    
    // 递归遍历所有子节点
    for (const auto& child : node->children) {
        collectAllContacts(child.second, result);
    }
}

std::vector<std:: shared_ptr<Contact>> Trie:: removeDuplicates(const std::vector<std::shared_ptr<Contact>>& contacts) const {
    std::set<int> seen;
    std::vector<std::shared_ptr<Contact>> unique_contacts;
    
    for (const auto& contact : contacts) {
        if (seen.find(contact->id) == seen.end()) {
            seen.insert(contact->id);
            unique_contacts. push_back(contact);
        }
    }
    
    return unique_contacts;
}

// 保持原有接口的兼容性
void Trie::insert(const std::string& key) {
    insertString(key);
}

void Trie::insertString(const std:: string& str, std::shared_ptr<Contact> contact) {
    auto chars = splitUTF8Characters(str);
    TrieNode* current = root;
    
    for (const auto& ch : chars) {
        if (current->children. find(ch) == current->children.end()) {
            current->children[ch] = new TrieNode();
        }
        current = current->children[ch];
        if (contact) {
            current->contacts.push_back(contact);
        }
    }
    current->isEndOfWord = true;
}

bool Trie::search(const std::string& key) {
    auto chars = splitUTF8Characters(key);
    TrieNode* current = root;
    
    for (const auto& ch :  chars) {
        if (current->children.find(ch) == current->children. end()) {
            return false;
        }
        current = current->children[ch];
    }
    return current->isEndOfWord;
}

bool Trie:: startsWith(const std::string& prefix) {
    auto chars = splitUTF8Characters(prefix);
    TrieNode* current = root;
    
    for (const auto& ch : chars) {
        if (current->children.find(ch) == current->children.end()) {
            return false;
        }
        current = current->children[ch];
    }
    return true;
}

void Trie::clear() {
    deleteNode(root);
    root = new TrieNode();
    contactCount = 0;
}

size_t Trie::getContactCount() const {
    return contactCount;
}

void Trie::printAllContacts() const {
    std::vector<std::shared_ptr<Contact>> allContacts;
    collectAllContacts(root, allContacts);
    auto uniqueContacts = removeDuplicates(allContacts);
    
    std:: cout << "=== 所有联系人 ===\n";
    for (const auto& contact : uniqueContacts) {
        std::cout << "ID: " << contact->id 
                  << ", 姓名:  " << contact->name 
                  << ", 学号:  " << contact->student_id 
                  << ", 电话: " << contact->phone << "\n";
    }
    std::cout << "总计: " << uniqueContacts.size() << " 人\n";
}