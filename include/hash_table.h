#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "sqlite_manager.h"
#include <vector>
#include <memory>
#include <functional>

class HashTable {
public:  
    // 构造函数和析构函数
    HashTable(size_t initial_capacity = 16);
    ~HashTable();
    
    // 主要操作
    bool insert(std::shared_ptr<Contact> contact);           // 插入联系人
    std::shared_ptr<Contact> find(int key);                  // 按ID查找
    std::shared_ptr<Contact> findByStudentId(const std:: string& student_id); // 按学号查找
    bool remove(int key);                                    // 删除联系人
    bool contains(int key);                                  // 检查是否存在
    
    // 去重功能
    bool isDuplicate(const Contact& contact);                // 检查重复
    std::vector<std::shared_ptr<Contact>> findDuplicates();  // 找出所有重复项
    size_t removeDuplicates();                              // 移除重复项
    
    // 统计和工具函数
    size_t size() const;                                    // 获取元素数量
    size_t capacity() const;                                // 获取容量
    double loadFactor() const;                              // 获取负载因子
    void clear();                                           // 清空哈希表
    
    // 调试和统计
    void printStatistics() const;                           // 打印统计信息
    void printDistribution() const;                         // 打印分布情况
    std::vector<std::shared_ptr<Contact>> getAllContacts(); // 获取所有联系人
    
private:
    // 哈希表节点
    struct HashNode {
        std::shared_ptr<Contact> contact;
        HashNode* next;
        
        HashNode(std::shared_ptr<Contact> c) : contact(c), next(nullptr) {}
    };
    
    std::vector<HashNode*> table;           // 哈希表数组
    size_t table_size;                      // 当前容量
    size_t element_count;                   // 元素数量
    
    static const double MAX_LOAD_FACTOR;    // 最大负载因子
    static const size_t MIN_CAPACITY;       // 最小容量
    
    // 内部辅助函数
    size_t hashFunction(int key) const;                     // 主哈希函数
    void resize();                                          // 动态扩容
    void insertHelper(std::shared_ptr<Contact> contact, std::vector<HashNode*>& target_table);
    
    // 冲突处理相关
    size_t getIndex(int key) const;                         // 获取索引位置
    HashNode* findNode(int key);                            // 查找节点
    
    // 统计函数
    size_t getChainLength(size_t index) const;              // 获取链长度
    size_t getMaxChainLength() const;                       // 获取最大链长度
    double getAverageChainLength() const;                   // 获取平均链长度
};

#endif // HASH_TABLE_H