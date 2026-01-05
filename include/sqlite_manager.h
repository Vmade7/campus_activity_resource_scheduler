#ifndef SQLITE_MANAGER_H
#define SQLITE_MANAGER_H

#include <string>
#include <vector>
#include <sqlite3.h>

struct Contact {
    int id;
    std::string name;
    std::string student_id;
    std::string phone;
    std::string email;
    std::string department;
    
    Contact(int id = 0, const std::string& name = "", 
            const std::string& student_id = "",
            const std::string& phone = "", 
            const std::string& email = "",
            const std::string& department = "")
        : id(id), name(name), student_id(student_id), 
          phone(phone), email(email), department(department) {}
    
    bool operator==(const Contact& other) const {
        return id == other.id;
    }
};

struct Activity {
    int id;
    std::string name;
    std:: string location;
    std::string start_time;
    std::string end_time;
    
    Activity(int id = 0, const std::string& name = "", const std::string& location = "",
             const std::string& start_time = "", const std::string& end_time = "")
        : id(id), name(name), location(location), start_time(start_time), end_time(end_time) {}
};

class SQLiteManager {
private:
    sqlite3* db;
    std::string db_path;

public:
    SQLiteManager(const std::string& path = "data/database.db");
    ~SQLiteManager();
    
    // 禁用拷贝（防止双重释放）
    SQLiteManager(const SQLiteManager&) = delete;
    SQLiteManager& operator=(const SQLiteManager&) = delete;
    
    // 基本功能
    bool init();
    bool isOpen() const;
    
    // 联系人操作
    bool addContact(const Contact& contact);
    std::vector<Contact> getAllContacts();
    bool deleteContact(int id);
    
    // 活动操作
    bool addActivity(const Activity& activity);
    std::vector<Activity> getAllActivities();
    bool deleteActivity(int id);
    
    // 工具
    void clearAll();
};

#endif // SQLITE_MANAGER_H