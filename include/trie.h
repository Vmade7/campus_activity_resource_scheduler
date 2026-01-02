#ifndef TRIE_H
#define TRIE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

// Contact结构体 - 联系人信息
struct Contact {
    int id;
    std::string name;
    std::string student_id;
    std:: string phone;
    std::string email;
    std::string department;
    
    Contact(int id, const std::string& name, const std::string& student_id, 
            const std::string& phone, const std:: string& email, 
            const std:: string& department = "")
        : id(id), name(name), student_id(student_id), 
          phone(phone), email(email), department(department) {}
    bool operator==(const Contact& other) const {
        return id == other.id;
    }
};

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