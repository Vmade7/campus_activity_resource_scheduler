#ifndef CONTACT_MANAGER_H
#define CONTACT_MANAGER_H

#include "data_manager.h"
#include "trie.h"
#include "hash_table.h"
#include <string>
#include <vector>
#include <memory>

class ContactManager {
private: 
    DataManager* data_manager;                      //改为指针（由外部注入）
    std::unique_ptr<Trie> name_index;              // 姓名前缀搜索索引
    std:: unique_ptr<HashTable> phone_index;        // 电话号码快速查找索引
    
    bool indices_built;                             // 索引是否已构建

public:
    //接收外部注入的DataManager
    ContactManager(DataManager* dm,
                  const std::string& backup_dir = "data/");
    
    // 保留向后兼容的构造函数（但标记为deprecated）
    ContactManager(const std::string& db_path = "data/database.db",
                  const std::string& backup_dir = "data/");
    ~ContactManager();
    
    // 禁用拷贝
    ContactManager(const ContactManager&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    
    // 初始化
    bool initialize();
    bool isReady() const;
    
    // 基本联系人操作
    bool addContact(const std::string& name, const std:: string& phone, const std::string& email);
    bool addContact(const Contact& contact);
    bool removeContact(int id);
    bool updateContact(const Contact& contact);
    
    // 高级查询功能
    std::vector<Contact> searchByName(const std::string& name_prefix);      // 前缀搜索
    Contact* findByPhone(const std::string& phone);                        // 电话查找
    Contact* findByEmail(const std::string& email);                        // 邮箱查找
    Contact* findById(int id);                                             // ID查找
    std::vector<Contact> getAllContacts();                                 // 获取所有联系人
    
    // 数据分析
    int getTotalCount();
    std::vector<std::string> getPopularNames(int limit = 5);              // 热门姓名统计
    bool hasDuplicatePhone(const std::string& phone);                     // 检查电话重复
    bool hasDuplicateEmail(const std::string& email);                     // 检查邮箱重复
    
    // 批量操作
    bool importContacts(const std::vector<Contact>& contacts);            // 批量导入
    bool exportContacts(const std::string& filename);                     // 导出到文件
    
    // 索引管理
    bool rebuildIndices();                                                 // 重建索引
    void printIndexStats();                                               // 打印索引统计
    
private:
    // 内部辅助方法
    void updateIndices(const Contact& contact);                           // 更新索引
    void removeFromIndices(const Contact& contact);                       // 从索引中移除
    bool validateContact(const Contact& contact);                         // 联系人验证
    Contact createContact(const std::string& name, const std::string& phone, const std::string& email);
};

#endif // CONTACT_MANAGER_H
