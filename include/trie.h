#ifndef TRIE_H
#define TRIE_H

#include "sqlite_manager.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class Trie {
public:
    Trie();
    ~Trie();
    
    void insertContact(std::shared_ptr<Contact> contact);
    std::vector<std:: shared_ptr<Contact>> searchByNamePrefix(const std::string& prefix);
    std::vector<std:: shared_ptr<Contact>> searchByStudentIdPrefix(const std::string& prefix);
    bool deleteContact(int contactId);

    void insert(const std::string& key);
    bool search(const std::string& key);
    bool startsWith(const std:: string& prefix);
    
    void clear();
    size_t getContactCount() const;
    void printAllContacts() const; 

private:
    struct TrieNode {
        std::unordered_map<std::string, TrieNode*> children;  // 改为string支持UTF-8
        std::vector<std:: shared_ptr<Contact>> contacts;       // 存储联系人指针
        bool isEndOfWord;
        
        TrieNode() : isEndOfWord(false) {}
    };
    
    TrieNode* root;
    size_t contactCount;
    
    // UTF-8处理函数
    std::vector<std::string> splitUTF8Characters(const std::string& str) const;
    bool isUTF8Start(unsigned char byte) const;
    int getUTF8CharLength(unsigned char byte) const;
    
    void collectAllContacts(TrieNode* node, std::vector<std::shared_ptr<Contact>>& result) const;
    void deleteNode(TrieNode* node);
    std::vector<std:: shared_ptr<Contact>> removeDuplicates(const std::vector<std::shared_ptr<Contact>>& contacts) const;

    void insertString(const std::string& str, std::shared_ptr<Contact> contact = nullptr);
};

#endif // TRIE_H