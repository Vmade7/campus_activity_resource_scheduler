#include "../include/hash_table.h"
#include <iostream>
#include <chrono>
#include <random>

void testBasicOperations() {
    std::cout << "=== 测试1: 基础操作 ===\n";
    
    HashTable hashTable;
    
    // 创建测试联系人
    auto contact1 = std::make_shared<Contact>(1, "张三", "2021001", "13800138001", "zhangsan@czu.edu. cn");
    auto contact2 = std::make_shared<Contact>(2, "李四", "2021002", "13800138002", "lisi@czu.edu.cn");
    auto contact3 = std::make_shared<Contact>(3, "王五", "2021003", "13800138003", "wangwu@czu.edu. cn");
    
    // 测试插入
    std::cout << "1. 插入联系人:\n";
    std::cout << "   插入张三: " << (hashTable.insert(contact1) ? "成功" : "失败") << "\n";
    std::cout << "   插入李四: " << (hashTable.insert(contact2) ? "成功" : "失败") << "\n";
    std::cout << "   插入王五: " << (hashTable.insert(contact3) ? "成功" : "失败") << "\n";
    
    hashTable.printStatistics();
    
    // 测试查找
    std::cout << "2. 查找联系人:\n";
    auto found1 = hashTable.find(2);
    std::cout << "   查找ID=2: " << (found1 ? found1->name : "未找到") << "\n";
    
    auto found2 = hashTable.findByStudentId("2021003");
    std::cout << "   查找学号=2021003: " << (found2 ? found2->name : "未找到") << "\n";
    
    auto found3 = hashTable.find(999);
    std::cout << "   查找ID=999: " << (found3 ? found3->name : "未找到") << "\n";
    
    // 测试包含
    std::cout << "3. 检查存在性:\n";
    std::cout << "   包含ID=1: " << (hashTable.contains(1) ? "是" : "否") << "\n";
    std::cout << "   包含ID=999: " << (hashTable.contains(999) ? "是" : "否") << "\n";
    
    // 测试删除
    std::cout << "4. 删除联系人:\n";
    std::cout << "   删除ID=2: " << (hashTable.remove(2) ? "成功" : "失败") << "\n";
    std::cout << "   删除后大小: " << hashTable.size() << "\n";
    
    hashTable.printStatistics();
}

void testDuplicateDetection() {
    std::cout << "=== 测试2: 重复检测 ===\n";
    
    HashTable hashTable;
    
    auto contact1 = std::make_shared<Contact>(1, "张三", "2021001", "13800138001", "zhangsan@czu.edu.cn");
    auto contact2 = std::make_shared<Contact>(2, "李四", "2021002", "13800138002", "lisi@czu. edu.cn");
    auto contact3 = std::make_shared<Contact>(1, "张三重复", "2021001", "13800138001", "duplicate@czu.edu.cn"); // ID重复
    auto contact4 = std::make_shared<Contact>(3, "王五", "2021002", "13800138003", "wangwu@czu. edu.cn"); // 学号重复
    
    hashTable.insert(contact1);
    hashTable.insert(contact2);
    
    std::cout << "1. 重复检测:\n";
    std::cout << "   contact3 (ID重复): " << (hashTable.isDuplicate(*contact3) ? "重复" : "不重复") << "\n";
    std:: cout << "   contact4 (学号重复): " << (hashTable.isDuplicate(*contact4) ? "重复" : "不重复") << "\n";
    
    // 插入重复项
    hashTable.insert(contact3); // ID重复，会更新
    hashTable.insert(contact4); // 学号重复，但ID不同，会插入
    
    std::cout << "2. 插入重复项后:\n";
    hashTable.printStatistics();
    
    // 查找重复项
    auto duplicates = hashTable.findDuplicates();
    std::cout << "3. 发现重复项:  " << duplicates.size() << " 个\n";
    for (const auto& dup : duplicates) {
        std::cout << "   " << dup->name << " (ID:" << dup->id << ", 学号:" << dup->student_id << ")\n";
    }
}

void testPerformance() {
    std::cout << "=== 测试3: 性能测试 ===\n";
    
    HashTable hashTable;
    
    const int TEST_SIZE = 10000;
    
    // 生成测试数据
    std::vector<std::shared_ptr<Contact>> test_contacts;
    test_contacts.reserve(TEST_SIZE);
    
    for (int i = 0; i < TEST_SIZE; i++) {
        auto contact = std::make_shared<Contact>(
            i, 
            "测试用户" + std::to_string(i),
            "2021" + std::to_string(i + 100000).substr(1), // 生成学号
            "138" + std::to_string(i + 10000000).substr(1), // 生成电话
            "user" + std::to_string(i) + "@czu.edu.cn"
        );
        test_contacts.push_back(contact);
    }
    
    // 测试插入性能
    auto start = std:: chrono::high_resolution_clock::now();
    for (const auto& contact : test_contacts) {
        hashTable.insert(contact);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto insert_time = std::chrono:: duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "1. 插入 " << TEST_SIZE << " 个联系人耗时: " << insert_time.count() << " ms\n";
    
    hashTable.printStatistics();
    
    // 测试查找性能
    std::random_device rd;
    std:: mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, TEST_SIZE - 1);
    
    const int SEARCH_COUNT = 1000;
    int found_count = 0;
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < SEARCH_COUNT; i++) {
        int random_id = dis(gen);
        if (hashTable.find(random_id) != nullptr) {
            found_count++;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    
    auto search_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "2. 随机查找 " << SEARCH_COUNT << " 次耗时: " << search_time.count() << " μs\n";
    std::cout << "   找到:  " << found_count << " 个\n";
    std::cout << "   平均每次查找:  " << (search_time.count() / SEARCH_COUNT) << " μs\n";
    
    // 显示分布情况
    hashTable.printDistribution();
}

void testResize() {
    std::cout << "=== 测试4: 动态扩容 ===\n";
    
    HashTable hashTable(4); // 从小容量开始
    
    std::cout << "1. 初始状态:\n";
    hashTable.printStatistics();
    
    // 逐步添加元素触发扩容
    for (int i = 0; i < 20; i++) {
        auto contact = std::make_shared<Contact>(
            i, "用户" + std::to_string(i), "202100" + std::to_string(i), 
            "138000" + std::to_string(i), "user" + std::to_string(i) + "@test.com"
        );
        
        hashTable.insert(contact);
        
        if (i == 3 || i == 7 || i == 15 || i == 19) { // 在关键点显示状态
            std::cout << "2. 插入 " << (i + 1) << " 个元素后:\n";
            hashTable.printStatistics();
        }
    }
}

int main() {
    testBasicOperations();
    std::cout << "\n";
    
    testDuplicateDetection();
    std::cout << "\n";
    
    testPerformance();
    std::cout << "\n";
    
    testResize();
    
    std::cout << "=== 哈希表测试完成 ===\n";
    return 0;
}