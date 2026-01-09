#include "../include/contact_manager.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>

//新的构造函数：接收外部注入的DataManager
ContactManager::ContactManager(DataManager* dm, const std::string& backup_dir) 
    : data_manager(dm), indices_built(false) {
    
    if (data_manager == nullptr) {
        std::cerr << "错误: DataManager不能为nullptr!" << std::endl;
    }
    name_index.reset(new Trie());
    phone_index.reset(new HashTable(64));
}

// 向后兼容的构造函数（不推荐使用，但保留）
ContactManager::ContactManager(const std::string& db_path, const std::string& backup_dir) 
    : indices_built(false) {
    
    // 创建新的DataManager（旧方式，应该用新构造函数）
    std::cerr << "警告: 使用已弃用的ContactManager构造函数！建议改用依赖注入。" << std::endl;
    data_manager = new DataManager(db_path, backup_dir, 100);
    name_index.reset(new Trie());
    phone_index.reset(new HashTable(64));
}

ContactManager::~ContactManager() {
    // 注意: 不释放data_manager，因为是外部注入的
}

bool ContactManager::initialize() {
    std::cout << "初始化联系人管理器..." << std::endl;
    
    // 初始化数据管理器
    if (!data_manager->initialize()) {
        std::cerr << "数据管理器初始化失败" << std::endl;
        return false;
    }
    
    // 构建索引
    if (!rebuildIndices()) {
        std::cerr << "索引构建失败" << std::endl;
        return false;
    }
    
    std::cout << "联系人管理器初始化成功" << std::endl;
    return true;
}

bool ContactManager::isReady() const {
    return data_manager && data_manager->isReady() && indices_built;
}

// 基本操作

bool ContactManager::addContact(const std::string& name, const std::string& phone, const std:: string& email) {
    Contact contact = createContact(name, phone, email);
    return addContact(contact);
}

bool ContactManager::addContact(const Contact& contact) {
    if (!isReady()) {
        std::cerr << "联系人管理器未就绪" << std::endl;
        return false;
    }
    
    if (! validateContact(contact)) {
        return false;
    }
    
    // 检查重复
    if (hasDuplicatePhone(contact.phone)) {
        std::cerr << "电话号码已存在:  " << contact.phone << std:: endl;
        return false;
    }
    
    if (hasDuplicateEmail(contact.email)) {
        std::cerr << "邮箱地址已存在: " << contact.email << std::endl;
        return false;
    }
    
    // 添加到数据存储
    if (!data_manager->addContact(contact)) {
        return false;
    }
    
    // 获取插入后的联系人（含自动生成的ID）
    auto contacts = data_manager->getAllContacts();
    if (! contacts.empty()) {
        const Contact& newContact = contacts. back();
        updateIndices(newContact);
        std::cout << "联系人已添加: " << newContact. name << " (ID: " << newContact.id << ")" << std::endl;
    }
    
    return true;
}

bool ContactManager::removeContact(int id) {
    if (!isReady()) return false;
    
    // 先获取联系人信息（用于从索引中移除）
    Contact* contact = data_manager->getContact(id);
    if (!contact) {
        std::cerr << "联系人不存在: ID=" << id << std::endl;
        return false;
    }
    
    Contact contactCopy = *contact;
    delete contact;
    
    // 从数据存储中删除
    if (!data_manager->deleteContact(id)) {
        return false;
    }
    
    // 从索引中移除
    removeFromIndices(contactCopy);
    
    std::cout << "联系人已删除: " << contactCopy.name << std::endl;
    return true;
}

bool ContactManager::updateContact(const Contact& contact) {
    if (!isReady() || !validateContact(contact)) return false;
    
    // 更新数据存储
    if (!data_manager->updateContact(contact)) {
        return false;
    }
    
    // 更新索引
    updateIndices(contact);
    
    std::cout << "联系人已更新: " << contact.name << std::endl;
    return true;
}

// 高级查询

std::vector<Contact> ContactManager::searchByName(const std:: string& name_prefix) {
    if (!isReady()) return {};
    
    auto results = name_index->searchByNamePrefix(name_prefix);
    std::cout << "按姓名前缀 '" << name_prefix << "' 搜索到 " << results.size() << " 个结果" << std::endl;
    
    std::vector<Contact> contacts;
    for (const auto& contactPtr : results) {
        contacts.push_back(*contactPtr);
    }
    return contacts;
}

Contact* ContactManager::findByPhone(const std::string& phone) {
    if (!isReady()) return nullptr;
    
    // 在所有联系人中查找（简化实现）
    auto contacts = data_manager->getAllContacts();
    for (const auto& contact :  contacts) {
        if (contact.phone == phone) {
            std::cout << "通过电话找到联系人: " << contact.name << std::endl;
            return new Contact(contact);
        }
    }
    
    return nullptr;
}

Contact* ContactManager::findByEmail(const std:: string& email) {
    if (!isReady()) return nullptr;
    
    auto contacts = data_manager->getAllContacts();
    for (const auto& contact : contacts) {
        if (contact.email == email) {
            std::cout << "通过邮箱找到联系人: " << contact.name << std::endl;
            return new Contact(contact);
        }
    }
    
    return nullptr;
}

Contact* ContactManager::findById(int id) {
    if (!isReady()) return nullptr;
    return data_manager->getContact(id);
}

std::vector<Contact> ContactManager::getAllContacts() {
    if (!isReady()) return {};
    return data_manager->getAllContacts();
}

// 数据分析

int ContactManager::getTotalCount() {
    if (!isReady()) return 0;
    return data_manager->getContactCount();
}

std::vector<std::string> ContactManager::getPopularNames(int limit) {
    if (!isReady()) return {};
    
    auto contacts = getAllContacts();
    std::unordered_map<std::string, int> nameCount;
    
    // 统计姓名出现频率
    for (const auto& contact : contacts) {
        nameCount[contact.name]++;
    }
    
    // 转换为vector并排序
    std::vector<std::pair<std::string, int>> nameFreq(nameCount.begin(), nameCount.end());
    std::sort(nameFreq.begin(), nameFreq.end(), 
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { 
                  return a.second > b.second; 
              });
    
    // 提取热门姓名
    std:: vector<std::string> popularNames;
    for (int i = 0; i < std::min(limit, (int)nameFreq.size()); ++i) {
        popularNames. push_back(nameFreq[i].first);
    }
    
    return popularNames;
}

bool ContactManager::hasDuplicatePhone(const std:: string& phone) {
    Contact* existing = findByPhone(phone);
    bool hasDuplicate = (existing != nullptr);
    if (existing) delete existing;
    return hasDuplicate;
}

bool ContactManager::hasDuplicateEmail(const std::string& email) {
    Contact* existing = findByEmail(email);
    bool hasDuplicate = (existing != nullptr);
    if (existing) delete existing;
    return hasDuplicate;
}

// 批量操作

bool ContactManager::importContacts(const std::vector<Contact>& contacts) {
    if (!isReady()) return false;
    
    int successCount = 0;
    int errorCount = 0;
    
    for (const auto& contact : contacts) {
        if (addContact(contact)) {
            successCount++;
        } else {
            errorCount++;
        }
    }
    
    std::cout << "联系人导入完成: 成功 " << successCount << " 个, 失败 " << errorCount << " 个" << std:: endl;
    return errorCount == 0;
}

bool ContactManager::exportContacts(const std::string& filename) {
    if (!isReady()) return false;
    return data_manager->backupAllData();
}

// 索引管理

bool ContactManager::rebuildIndices() {
    std::cout << "重建联系人索引..." << std:: endl;
    
    // 清空现有索引
    name_index->clear();
    phone_index->clear();
    
    // 获取所有联系人
    auto contacts = data_manager->getAllContacts();
    
    // 重建索引
    for (const auto& contact : contacts) {
        updateIndices(contact);
    }
    
    indices_built = true;
    std::cout << "索引重建完成，共处理 " << contacts.size() << " 个联系人" << std::endl;
    return true;
}

void ContactManager::printIndexStats() {
    if (!isReady()) {
        std::cout << "联系人管理器未就绪" << std::endl;
        return;
    }
    
    std::cout << "=== 联系人索引统计 ===" << std::endl;
    std::cout << "总联系人数: " << getTotalCount() << std::endl;
    std::cout << "姓名索引: " << name_index->getContactCount() << " 条记录" << std::endl;
    std::cout << "电话索引: " << phone_index->size() << " 条记录" << std::endl;
}

// 私有辅助方法

void ContactManager::updateIndices(const Contact& contact) {
    if (contact.id <= 0) return;
    
    // 更新姓名索引
    auto contactPtr = std::make_shared<Contact>(contact);
    name_index->insertContact(contactPtr);
    
    // 更新电话索引（简化实现：这里应该用专门的电话索引）
    phone_index->insert(contactPtr);
}

void ContactManager::removeFromIndices(const Contact& contact) {
    // 简化实现：实际项目中需要从索引中精确移除
    std::cout << "从索引中移除联系人:  " << contact.name << std:: endl;
}

bool ContactManager::validateContact(const Contact& contact) {
    if (contact.name.empty()) {
        std::cerr << "联系人姓名不能为空" << std::endl;
        return false;
    }
    
    if (contact.phone.empty()) {
        std::cerr << "联系人电话不能为空" << std::endl;
        return false;
    }
    
    if (contact.email.empty()) {
        std::cerr << "联系人邮箱不能为空" << std::endl;
        return false;
    }
    
    // 简单的邮箱格式验证
    if (contact.email.find('@') == std::string::npos) {
        std::cerr << "邮箱格式不正确" << std::endl;
        return false;
    }
    
    return true;
}

Contact ContactManager::createContact(const std::string& name, const std::string& phone, const std::string& email) {
    return Contact(0, name, phone, email);  // ID由数据库自动生成
}