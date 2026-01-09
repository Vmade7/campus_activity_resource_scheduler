#include "../include/auth_manager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>

AuthManager::AuthManager(const std:: string& db_path, const std::string& secret)
    : jwt_secret(secret), token_expire_hours(24) {
    
    db_manager.reset(new SQLiteManager(db_path));
}

AuthManager::~AuthManager() = default;

bool AuthManager::initialize() {
    std::cout << "初始化认证管理器..." << std::endl;
    
    if (!db_manager->init()) {
        std::cerr << "认证数据库初始化失败" << std::endl;
        return false;
    }
    
    if (!initializeDatabase()) {
        std::cerr << "用户表创建失败" << std::endl;
        return false;
    }
    
    // 创建默认管理员账户
    if (getUserByUsername("admin") == nullptr) {
        registerUser("admin", "admin123", "系统管理员", "admin@campus.edu", "系统管理", UserRole::ADMIN);
        std::cout << "创建默认管理员账户:  admin/admin123" << std:: endl;
    }
    
    std::cout << "认证管理器初始化成功" << std:: endl;
    return true;
}

bool AuthManager::initializeDatabase() {
    // 这里需要直接执行SQL，因为SQLiteManager可能不支持用户表
    // 简化实现：假设我们扩展了SQLiteManager
    
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role INTEGER NOT NULL,
            real_name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            department TEXT,
            is_active BOOLEAN DEFAULT 1,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            last_login TIMESTAMP
        );
    )";
    
    // 注意：这里需要SQLiteManager支持执行原生SQL
    // 或者我们需要直接操作sqlite3
    
    sqlite3* db;
    int rc = sqlite3_open(db_manager->getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "无法打开认证数据库: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    char* errMsg;
    rc = sqlite3_exec(db, create_users_table, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "创建用户表失败: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_close(db);
    return true;
}

bool AuthManager::registerUser(const std::string& username, const std::string& password,
                              const std::string& real_name, const std::string& email,
                              const std::string& department, UserRole role) {
    
    // 检查用户名是否已存在
    if (getUserByUsername(username) != nullptr) {
        std::cerr << "用户名已存在: " << username << std::endl;
        return false;
    }
    
    // 密码强度验证
    if (password.length() < 6) {
        std::cerr << "密码长度至少6位" << std::endl;
        return false;
    }
    
    std::string password_hash = hashPassword(password);
    
    // 插入用户到数据库
    sqlite3* db;
    int rc = sqlite3_open(db_manager->getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) return false;
    
    const char* sql = R"(
        INSERT INTO users (username, password_hash, role, real_name, email, department)
        VALUES (?, ?, ?, ?, ?, ? );
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username. c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(role));
    sqlite3_bind_text(stmt, 4, real_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, department.c_str(), -1, SQLITE_STATIC);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (success) {
        std::cout << "用户注册成功: " << username << std::endl;
    }
    
    return success;
}

std::string AuthManager::authenticate(const std::string& username, const std::string& password) {
    User* user = getUserByUsername(username);
    if (!user) {
        std::cerr << "用户不存在: " << username << std::endl;
        return "";
    }
    
    if (!user->is_active) {
        std::cerr << "用户账户已被禁用: " << username << std::endl;
        delete user;
        return "";
    }
    
    if (!verifyPassword(password, user->password_hash)) {
        std::cerr << "密码错误: " << username << std:: endl;
        delete user;
        return "";
    }
    
    // 更新最后登录时间
    updateLastLogin(user->id);
    
    // 生成JWT token
    std::string token = generateJWT(*user);
    
    // 缓存token
    JWTPayload payload;
    if (parseJWT(token, payload)) {
        active_tokens[token] = payload;
    }
    
    std::cout << "用户登录成功: " << username << " (" << 
                 (user->role == UserRole:: ADMIN ? "管理员" : "学生") << ")" << std::endl;
    
    delete user;
    return token;
}

bool AuthManager::validateToken(const std::string& token) {
    // 首先检查缓存
    auto it = active_tokens.find(token);
    if (it != active_tokens.end()) {
        // 检查是否过期
        auto now = std::chrono::system_clock::now();
        if (now < it->second.expires_at) {
            return true;
        } else {
            // 删除过期token
            active_tokens.erase(it);
            return false;
        }
    }
    
    // 解析JWT
    JWTPayload payload;
    if (parseJWT(token, payload)) {
        auto now = std::chrono::system_clock::now();
        if (now < payload.expires_at) {
            // 重新缓存有效token
            active_tokens[token] = payload;
            return true;
        }
    }
    
    return false;
}

JWTPayload* AuthManager:: getTokenPayload(const std:: string& token) {
    if (! validateToken(token)) {
        return nullptr;
    }
    
    auto it = active_tokens. find(token);
    if (it != active_tokens.end()) {
        return &(it->second);
    }
    
    return nullptr;
}

std::string AuthManager::generateJWT(const User& user) {
    // 创建JWT头部
    std::string header = createJWTHeader();
    
    // 创建JWT载荷
    std::string payload = createJWTPayload(user);
    
    // 创建签名
    std::string signature = createJWTSignature(header, payload);
    
    return base64Encode(header) + "." + base64Encode(payload) + "." + signature;
}

User* AuthManager::getUserByUsername(const std::string& username) {
    sqlite3* db;
    int rc = sqlite3_open(db_manager->getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) return nullptr;
    
    const char* sql = "SELECT id, username, password_hash, role, real_name, email, department, is_active FROM users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = new User();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
        user->real_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        user->department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->is_active = sqlite3_column_int(stmt, 7) != 0;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return user;
}

// 私有辅助方法实现
std::string AuthManager:: hashPassword(const std::string& password) {
    // 使用SHA-256哈希密码（实际项目中应该使用bcrypt等更安全的方法）
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std:: hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool AuthManager::verifyPassword(const std::string& password, const std:: string& hash) {
    return hashPassword(password) == hash;
}

std::string AuthManager::createJWTHeader() {
    return R"({"alg":"HS256","typ":"JWT"})";
}

std::string AuthManager::createJWTPayload(const User& user) {
    auto now = std::chrono::system_clock::now();
    auto expires = now + std::chrono::hours(token_expire_hours);
    
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto exp_time_t = std::chrono::system_clock::to_time_t(expires);
    
    std::ostringstream payload;
    payload << "{"
            << "\"user_id\":" << user.id << ","
            << "\"username\":\"" << user.username << "\","
            << "\"role\":" << static_cast<int>(user.role) << ","
            << "\"real_name\":\"" << user. real_name << "\","
            << "\"iat\":" << now_time_t << ","
            << "\"exp\":" << exp_time_t
            << "}";
    
    return payload.str();
}

std::string AuthManager::createJWTSignature(const std::string& header, const std:: string& payload) {
    std::string data = base64Encode(header) + "." + base64Encode(payload);
    
    unsigned char* digest;
    unsigned int digest_len;
    
    digest = HMAC(EVP_sha256(), jwt_secret.c_str(), jwt_secret.length(),
                  reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
                  nullptr, &digest_len);
    
    return base64Encode(std::string(reinterpret_cast<char*>(digest), digest_len));
}

void AuthManager::updateLastLogin(int user_id) {
    sqlite3* db;
    int rc = sqlite3_open(db_manager->getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) return;
    
    const char* sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ? ";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

// Base64编码实现
std::string AuthManager::base64Encode(const std::string& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return "";
    
    BIO* b64 = BIO_new(BIO_f_base64());
    if (!b64) {
        BIO_free(bio);
        return "";
    }
    
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    if (BIO_write(bio, data.c_str(), static_cast<int>(data.length())) <= 0) {
        BIO_free_all(bio);
        return "";
    }
    
    if (BIO_flush(bio) <= 0) {
        BIO_free_all(bio);
        return "";
    }
    
    BUF_MEM* buf_mem;
    BIO_get_mem_ptr(bio, &buf_mem);
    std::string result(buf_mem->data, buf_mem->length);
    
    BIO_free_all(bio);
    return result;
}

void AuthManager::cleanExpiredTokens() {
    auto now = std::chrono::system_clock::now();
    
    for (auto it = active_tokens.begin(); it != active_tokens.end();) {
        if (now >= it->second.expires_at) {
            it = active_tokens.erase(it);
        } else {
            ++it;
        }
    }
}

// 实现 parseJWT（带签名验证）
bool AuthManager::parseJWT(const std::string& token, JWTPayload& payload) {
    // JWT分割为三部分：header.payload.signature
    size_t dot1 = token.find('.');
    if (dot1 == std::string::npos) return false;
    
    size_t dot2 = token.find('.', dot1 + 1);
    if (dot2 == std::string::npos) return false;
    
    // 提取三个部分
    std::string header_b64 = token.substr(0, dot1);
    std::string payload_b64 = token.substr(dot1 + 1, dot2 - dot1 - 1);
    std::string signature_b64 = token.substr(dot2 + 1);
    
    //新增：验证签名
    //重新计算签名：HMAC(SHA256, secret, header.payload)
    std::string data_to_sign = header_b64 + "." + payload_b64;
    
    unsigned char* computed_digest;
    unsigned int digest_len;
    
    computed_digest = HMAC(EVP_sha256(), jwt_secret.c_str(), jwt_secret.length(),
                          reinterpret_cast<const unsigned char*>(data_to_sign.c_str()), 
                          data_to_sign.length(),
                          nullptr, &digest_len);
    
    std::string computed_signature = base64Encode(std::string(reinterpret_cast<char*>(computed_digest), digest_len));
    
    // 比较签名：使用恒定时间比较防止时序攻击
    if (signature_b64.length() != computed_signature.length()) {
        return false;
    }
    
    int signature_valid = 0;
    for (size_t i = 0; i < signature_b64.length(); ++i) {
        signature_valid |= (signature_b64[i] ^ computed_signature[i]);
    }
    
    if (signature_valid != 0) {
        std::cerr << "JWT签名验证失败！" << std::endl;
        return false;  // 签名不匹配
    }
    
    //签名验证通过，继续解析载荷
    std::string payload_str = base64Decode(payload_b64);
    
    // 简化的JSON解析（提取user_id、username、role、exp）
    size_t user_id_pos = payload_str.find("\"user_id\":");
    if (user_id_pos != std::string::npos) {
        size_t start = user_id_pos + 10;
        size_t comma = payload_str.find(',', start);
        payload.user_id = std::stoi(payload_str.substr(start, comma - start));
    }
    
    size_t username_pos = payload_str.find("\"username\":\"");
    if (username_pos != std::string::npos) {
        size_t start = username_pos + 12;
        size_t end = payload_str.find('\"', start);
        payload.username = payload_str.substr(start, end - start);
    }
    
    size_t role_pos = payload_str.find("\"role\":");
    if (role_pos != std::string::npos) {
        size_t start = role_pos + 7;
        size_t comma = payload_str.find(',', start);
        int role_int = std::stoi(payload_str.substr(start, comma - start));
        payload.role = static_cast<UserRole>(role_int);
    }
    
    size_t exp_pos = payload_str.find("\"exp\":");
    if (exp_pos != std::string::npos) {
        size_t start = exp_pos + 6;
        size_t end = payload_str.find_first_not_of("0123456789", start);
        time_t exp_time = std::stoll(payload_str.substr(start, end - start));
        payload.expires_at = std::chrono::system_clock::from_time_t(exp_time);
    }
    
    return true;
}

// 实现 base64Decode
std::string AuthManager::base64Decode(const std::string& data) {
    BIO* bio = BIO_new_mem_buf(data.c_str(), static_cast<int>(data.length()));
    if (!bio) return "";
    
    BIO* b64 = BIO_new(BIO_f_base64());
    if (!b64) {
        BIO_free(bio);
        return "";
    }
    
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    // 使用 vector 替代 VLA（Variable Length Array）
    std::vector<char> buffer(data.length() + 1);
    int decoded_length = BIO_read(bio, buffer.data(), static_cast<int>(data.length()));
    
    BIO_free_all(bio);
    
    if (decoded_length > 0) {
        return std::string(buffer.data(), decoded_length);
    }
    return "";
}

// 实现 getUserById
User* AuthManager::getUserById(int user_id) {
    sqlite3* db;
    int rc = sqlite3_open(db_manager->getDbPath().c_str(), &db);
    if (rc != SQLITE_OK) return nullptr;
    
    const char* sql = "SELECT id, username, password_hash, role, real_name, email, department, is_active FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = new User();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->role = static_cast<UserRole>(sqlite3_column_int(stmt, 3));
        user->real_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        user->department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        user->is_active = sqlite3_column_int(stmt, 7) != 0;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return user;
}