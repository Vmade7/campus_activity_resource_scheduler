#include "../include/sqlite_manager.h"
#include <iostream>

SQLiteManager::SQLiteManager(const std:: string& path) : db(nullptr), db_path(path) {}

SQLiteManager::~SQLiteManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool SQLiteManager::init() {
    // 打开数据库
    if (sqlite3_open(db_path. c_str(), &db) != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // 创建联系人表
    const char* createContacts = R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT NOT NULL,
            email TEXT NOT NULL
        );
    )";
    
    // 创建活动表
    const char* createActivities = R"(
        CREATE TABLE IF NOT EXISTS activities (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            location TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL
        );
    )";
    
    char* errMsg = nullptr;
    
    // 执行建表语句
    if (sqlite3_exec(db, createContacts, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "创建联系人表失败: " << errMsg << std:: endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    if (sqlite3_exec(db, createActivities, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "创建活动表失败: " << errMsg << std:: endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    std::cout << "数据库初始化成功" << std::endl;
    return true;
}

bool SQLiteManager::isOpen() const {
    return db != nullptr;
}

bool SQLiteManager::addContact(const Contact& contact) {
    if (!isOpen()) return false;
    
    // 简单验证
    if (contact.name.empty() || contact.phone.empty() || contact.email.empty()) {
        std:: cerr << "联系人信息不完整" << std::endl;
        return false;
    }
    
    const char* sql = "INSERT INTO contacts (name, phone, email) VALUES (?, ?, ? );";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, contact.name. c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contact.phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, contact. email.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    if (!success) {
        std::cerr << "插入联系人失败: " << sqlite3_errmsg(db) << std::endl;
    }
    
    return success;
}

std::vector<Contact> SQLiteManager::getAllContacts() {
    std::vector<Contact> contacts;
    if (!isOpen()) return contacts;
    
    const char* sql = "SELECT id, name, phone, email FROM contacts;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std:: cerr << "查询联系人失败:  " << sqlite3_errmsg(db) << std::endl;
        return contacts;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Contact contact(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
        );
        contacts.push_back(contact);
    }
    
    sqlite3_finalize(stmt);
    return contacts;
}

bool SQLiteManager::deleteContact(int id) {
    if (!isOpen()) return false;
    
    const char* sql = "DELETE FROM contacts WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

bool SQLiteManager::addActivity(const Activity& activity) {
    if (! isOpen()) return false;
    
    // 简单验证
    if (activity.name.empty() || activity.location. empty()) {
        std:: cerr << "活动信息不完整" << std:: endl;
        return false;
    }
    
    const char* sql = "INSERT INTO activities (name, location, start_time, end_time) VALUES (?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, activity. name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, activity.location.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, activity.start_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, activity. end_time.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    if (!success) {
        std::cerr << "插入活动失败: " << sqlite3_errmsg(db) << std::endl;
    }
    
    return success;
}

std:: vector<Activity> SQLiteManager::getAllActivities() {
    std::vector<Activity> activities;
    if (!isOpen()) return activities;
    
    const char* sql = "SELECT id, name, location, start_time, end_time FROM activities;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "查询活动失败: " << sqlite3_errmsg(db) << std::endl;
        return activities;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Activity activity(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        );
        activities.push_back(activity);
    }
    
    sqlite3_finalize(stmt);
    return activities;
}

bool SQLiteManager::deleteActivity(int id) {
    if (!isOpen()) return false;
    
    const char* sql = "DELETE FROM activities WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

void SQLiteManager::clearAll() {
    if (!isOpen()) return;
    
    sqlite3_exec(db, "DELETE FROM contacts", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "DELETE FROM activities", nullptr, nullptr, nullptr);
}