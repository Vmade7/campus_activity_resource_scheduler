#ifndef HTTP_SERVER_AUTH_H
#define HTTP_SERVER_AUTH_H

#include "http_server.h"
#include "auth_manager.h"
#include "contact_manager.h"
#include "activity_manager.h"
#include "conflict_detector.h"
#include <memory>
#include <mutex>
#include <chrono>

// 扩展HttpRequest支持认证信息
struct AuthenticatedRequest : public HttpRequest {
    bool is_authenticated = false;
    std::unique_ptr<User> current_user = nullptr;
    std::string auth_token;
    
    AuthenticatedRequest() = default;
    AuthenticatedRequest(const HttpRequest& base_request) : HttpRequest(base_request) {}
};

// 认证中间件
class AuthMiddleware {
private:
    AuthManager* auth_manager;
    
    // 速率限制相关成员
    mutable std::mutex rate_limit_mutex;
    std::map<std::string, int> login_attempts;  // IP -> 失败次数
    std::map<std::string, std::chrono::system_clock::time_point> lockout_time;  // IP -> 锁定时间
    
    // 速率限制常数
    static const int MAX_LOGIN_ATTEMPTS;
    static const int LOCKOUT_DURATION_SECONDS;
    static const int RESET_WINDOW_SECONDS;

public:
    AuthMiddleware(AuthManager* auth_mgr) : auth_manager(auth_mgr) {}
    
    // 验证请求中的JWT token
    bool authenticate(AuthenticatedRequest& request);
    
    // 检查用户权限
    bool authorize(const AuthenticatedRequest& request, UserRole required_role);
    
    // 从请求头提取token
    std::string extractToken(const HttpRequest& request);
    
    // 检查登录速率限制
    bool checkRateLimit(const std::string& client_ip, std::string& error_message);
    
    // 记录登录失败
    void recordLoginFailure(const std::string& client_ip);
    
    // 清除登录失败记录
    void clearLoginFailure(const std::string& client_ip);
};

// 认证相关的路由处理器
class AuthRoutes {
private: 
    AuthManager* auth_manager;

public:
    AuthRoutes(AuthManager* auth_mgr) : auth_manager(auth_mgr) {}
    
    // 认证API路由
    HttpResponse handleLogin(const HttpRequest& request);
    HttpResponse handleRegister(const HttpRequest& request);
    HttpResponse handleLogout(const HttpRequest& request);
    HttpResponse handleProfile(const HttpRequest& request);
    HttpResponse handleRefreshToken(const HttpRequest& request);
    
    // 用户管理API（管理员专用）
    HttpResponse handleGetUsers(const HttpRequest& request);
    HttpResponse handleCreateUser(const HttpRequest& request);
    HttpResponse handleDeleteUser(const HttpRequest& request);
};

// 完整的认证HTTP服务器
class AuthenticatedHttpServer {
private:
    std::unique_ptr<AuthManager> auth_manager;
    std::unique_ptr<AuthMiddleware> auth_middleware;
    std::unique_ptr<AuthRoutes> auth_routes;
    std::unique_ptr<ContactManager> contact_manager;
    std::unique_ptr<ActivityManager> activity_manager;
    std::unique_ptr<ConflictDetector> conflict_detector;
    
    int port;
    std::atomic<bool> running;
    std::unique_ptr<std::thread> server_thread;
    
    // 路由映射
    std::map<std::string, std::function<HttpResponse(const AuthenticatedRequest&)>> protected_routes;
    std::map<std::string, std::function<HttpResponse(const HttpRequest&)>> public_routes;

public:
    AuthenticatedHttpServer(int server_port = 8080);
    ~AuthenticatedHttpServer();
    
    // 禁用拷贝
    AuthenticatedHttpServer(const AuthenticatedHttpServer&) = delete;
    AuthenticatedHttpServer& operator=(const AuthenticatedHttpServer&) = delete;
    
    // 服务器控制
    bool initialize();
    bool start();
    void stop();
    bool isRunning() const;
    
    // 路由注册
    void registerPublicRoute(const std::string& path, 
                           std::function<HttpResponse(const HttpRequest&)> handler);
    void registerProtectedRoute(const std::string& path, 
                              std::function<HttpResponse(const AuthenticatedRequest&)> handler);
    
private:
    // 服务器核心
    void serverLoop();
    HttpResponse handleRequest(const HttpRequest& request);
    void setupRoutes();
    
    // 辅助方法（添加声明）
    HttpRequest parseHttpRequest(const std::string& raw_request);
    std::string buildHttpResponse(const HttpResponse& response);
    
    // API路由处理器
    // 联系人API
    HttpResponse handleGetContacts(const AuthenticatedRequest& request);
    HttpResponse handleCreateContact(const AuthenticatedRequest& request);
    HttpResponse handleUpdateContact(const AuthenticatedRequest& request);
    HttpResponse handleDeleteContact(const AuthenticatedRequest& request);
    HttpResponse handleSearchContacts(const AuthenticatedRequest& request);
    
    // 活动API
    HttpResponse handleGetActivities(const AuthenticatedRequest& request);
    HttpResponse handleCreateActivity(const AuthenticatedRequest& request);
    HttpResponse handleUpdateActivity(const AuthenticatedRequest& request);
    HttpResponse handleDeleteActivity(const AuthenticatedRequest& request);
    
    // 资源调度API（管理员专用）
    HttpResponse handleGetSchedule(const AuthenticatedRequest& request);
    HttpResponse handleCheckConflict(const AuthenticatedRequest& request);
    HttpResponse handleCreateReservation(const AuthenticatedRequest& request);
    
    // 系统API
    HttpResponse handleGetStats(const AuthenticatedRequest& request);
    HttpResponse handleBackupData(const AuthenticatedRequest& request);
    
    // 静态文件服务
    HttpResponse serveStaticFile(const std::string& path);
    
    // 工具方法
    std::string buildJsonResponse(const std::map<std::string, std::string>& data);
    std::string buildErrorResponse(const std::string& error, int code = 400);
    bool matchRoutePattern(const std::string& pattern, const std::string& path);
};

#endif // HTTP_SERVER_AUTH_H