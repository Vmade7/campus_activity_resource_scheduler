#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include "sqlite_manager.h"
#include <string>
#include <map>
#include <memory>
#include <chrono>

// 用户角色枚举
enum class UserRole {
    STUDENT,
    ADMIN
};

// 用户信息结构
struct User {
    int id;
    std::string username;
    std::string password_hash;  // 存储密码哈希
    UserRole role;
    std:: string real_name;
    std::string email;
    std::string department;
    bool is_active;
    std:: chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_login;
    
    User() : id(0), role(UserRole::STUDENT), is_active(true) {}
};

// JWT载荷结构
struct JWTPayload {
    int user_id;
    std:: string username;
    UserRole role;
    std::chrono::system_clock::time_point issued_at;
    std::chrono::system_clock::time_point expires_at;
};

class AuthManager {
private:
    std::unique_ptr<SQLiteManager> db_manager;
    std::string jwt_secret;                    // JWT密钥
    std::map<std::string, JWTPayload> active_tokens;  // 活跃token缓存
    int token_expire_hours;                   // token有效期（小时）

public:
    AuthManager(const std::string& db_path = "data/auth.db", 
               const std::string& secret = "campus_scheduler_secret");
    ~AuthManager();
    
    // 禁用拷贝
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;
    
    // 初始化
    bool initialize();
    
    // 用户管理
    bool registerUser(const std::string& username, const std::string& password,
                     const std::string& real_name, const std::string& email,
                     const std::string& department, UserRole role = UserRole::STUDENT);
    
    bool changePassword(int user_id, const std::string& old_password, const std::string& new_password);
    bool updateUserInfo(const User& user);
    User* getUserById(int user_id);
    User* getUserByUsername(const std:: string& username);
    
    // 认证相关
    std::string authenticate(const std::string& username, const std::string& password);  // 返回JWT token
    bool validateToken(const std::string& token);
    JWTPayload* getTokenPayload(const std::string& token);
    bool revokeToken(const std:: string& token);
    
    // JWT操作
    std::string generateJWT(const User& user);
    bool parseJWT(const std::string& token, JWTPayload& payload);
    
    // 工具方法
    void cleanExpiredTokens();
    int getActiveUserCount();
    std::vector<User> getRecentUsers(int limit = 10);

private:
    // 内部辅助方法
    std::string hashPassword(const std:: string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string base64Encode(const std::string& data);
    std::string base64Decode(const std:: string& data);
    std::string createJWTHeader();
    std::string createJWTPayload(const User& user);
    std::string createJWTSignature(const std::string& header, const std::string& payload);
    bool initializeDatabase();
    void updateLastLogin(int user_id);
};

#endif // AUTH_MANAGER_H