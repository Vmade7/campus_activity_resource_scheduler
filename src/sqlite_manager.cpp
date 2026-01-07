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
            student_id TEXT,
            phone TEXT NOT NULL,
            email TEXT NOT NULL,
            department TEXT
        );
    )";
    
    // 创建活动表
    const char* createActivities = R"(
        CREATE TABLE IF NOT EXISTS activities (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            location TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL,
            max_participants INTEGER DEFAULT 0,
            current_participants INTEGER DEFAULT 0,
            category TEXT,
            status TEXT DEFAULT 'upcoming',
            created_by TEXT,
            created_at TEXT DEFAULT (datetime('now')),
            updated_at TEXT DEFAULT (datetime('now'))
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

const std::string& SQLiteManager::getDbPath() const {
    return db_path;
}

bool SQLiteManager::addContact(const Contact& contact) {
    if (!isOpen()) return false;
    
    // 简单验证
    if (contact.name.empty() || contact.phone.empty() || contact.email.empty()) {
        std:: cerr << "联系人信息不完整" << std::endl;
        return false;
    }
    
    const char* sql = "INSERT INTO contacts (name, student_id, phone, email, department) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, contact.name. c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contact.student_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, contact.phone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, contact. email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, contact.department.c_str(), -1, SQLITE_STATIC);
    
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
    
    const char* sql = "SELECT id, name, student_id, phone, email, department FROM contacts;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std:: cerr << "查询联系人失败:  " << sqlite3_errmsg(db) << std::endl;
        return contacts;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* student_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        Contact contact(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            student_id ? student_id : "",
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            department ? department : ""
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
    
    const char* sql = "INSERT INTO activities (name, description, location, start_time, end_time, max_participants, current_participants, category, status, created_by) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "准备语句失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, activity. name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, "", -1, SQLITE_STATIC); // description - 暂时为空
    sqlite3_bind_text(stmt, 3, activity.location.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, activity.start_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, activity. end_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, 0); // max_participants - 默认0
    sqlite3_bind_int(stmt, 7, 0); // current_participants - 默认0
    sqlite3_bind_text(stmt, 8, "", -1, SQLITE_STATIC); // category - 暂时为空
    sqlite3_bind_text(stmt, 9, "upcoming", -1, SQLITE_STATIC); // status
    sqlite3_bind_text(stmt, 10, "system", -1, SQLITE_STATIC); // created_by
    
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
    
    const char* sql = "SELECT id, name, location, start_time, end_time, max_participants, current_participants, category, status FROM activities;";
    
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
        // 注意：Activity结构体可能需要扩展以支持更多字段
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