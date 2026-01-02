#include "../include/trie.h"
#include <iostream>

int main() {
    Trie trie;
    
    // 创建测试联系人
    auto contact1 = std::make_shared<Contact>(1, "张三", "2021001", "13800138000", "zhangsan@czu.edu. cn");
    auto contact2 = std::make_shared<Contact>(2, "张伟", "2021002", "13800138001", "zhangwei@czu.edu.cn");
    auto contact3 = std::make_shared<Contact>(3, "李四", "2021003", "13800138002", "lisi@czu. edu.cn");
    
    // 插入联系人
    trie.insertContact(contact1);
    trie. insertContact(contact2);
    trie.insertContact(contact3);
    
    // 测试中文前缀搜索
    auto results = trie.searchByNamePrefix("张");
    std::cout << "姓'张'的联系人有 " << results.size() << " 个\n";
    
    // 测试学号前缀搜索
    auto results2 = trie.searchByStudentIdPrefix("2021");
    std::cout << "学号以'2021'开头的联系人有 " << results2.size() << " 个\n";
    
    return 0;
}