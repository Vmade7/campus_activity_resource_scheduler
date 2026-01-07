#include "../include/http_server_auth.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// === AuthMiddleware 实现 ===

bool AuthMiddleware::authenticate(AuthenticatedRequest& request) {
    std::string token = extractToken(request);
    if (token.empty()) {
        return false;
    }
    
    if (! auth_manager->validateToken(token)) {
        return false;
    }
    
    JWTPayload* payload = auth_manager->getTokenPayload(token);
    if (!payload) {
        return false;
    }
    
    // 获取用户信息
    User* user = auth_manager->getUserById(payload->user_id);
    if (!user || !user->is_active) {
        return false;
    }
    
    request.is_authenticated = true;
    request.current_user = std::unique_ptr<User>(user);
    request.auth_token = token;
    
    return true;
}

bool AuthMiddleware::authorize(const AuthenticatedRequest& request, UserRole required_role) {
    if (!request. is_authenticated || !request.current_user) {
        return false;
    }
    
    // 管理员拥有所有权限
    if (request.current_user->role == UserRole:: ADMIN) {
        return true;
    }
    
    // 检查是否满足所需角色
    return request.current_user->role == required_role;
}

std::string AuthMiddleware::extractToken(const HttpRequest& request) {
    auto auth_header = request. headers.find("Authorization");
    if (auth_header == request.headers.end()) {
        return "";
    }
    
    const std::string& auth_value = auth_header->second;
    if (auth_value. length() > 7 && auth_value.substr(0, 7) == "Bearer ") {
        return auth_value.substr(7);
    }
    
    return "";
}

// === AuthRoutes 实现 ===

HttpResponse AuthRoutes::handleLogin(const HttpRequest& request) {
    if (request.method != "POST") {
        HttpResponse response(405, "Method Not Allowed");
        response.setJson(buildErrorResponse("Only POST method allowed"));
        return response;
    }
    
    // 解析请求体中的用户名和密码
    std::regex username_regex(R"("username"\s*:\s*"([^"]+)")");
    std::regex password_regex(R"("password"\s*:\s*"([^"]+)")");
    
    std::smatch username_match, password_match;
    
    if (! std::regex_search(request.body, username_match, username_regex) ||
        !std:: regex_search(request.body, password_match, password_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing username or password"));
        return response;
    }
    
    std::string username = username_match[1].str();
    std::string password = password_match[1].str();
    
    // 验证用户
    std::string token = auth_manager->authenticate(username, password);
    if (token.empty()) {
        HttpResponse response(401, "Unauthorized");
        response.setJson(buildErrorResponse("Invalid username or password"));
        return response;
    }
    
    // 获取用户信息
    User* user = auth_manager->getUserByUsername(username);
    if (!user) {
        HttpResponse response(500, "Internal Server Error");
        response.setJson(buildErrorResponse("Failed to retrieve user information"));
        return response;
    }
    
    // 构建成功响应
    std::ostringstream json_response;
    json_response << "{"
                  << "\"success\": true,"
                  << "\"token\": \"" << token << "\","
                  << "\"user\": {"
                  << "\"id\": " << user->id << ","
                  << "\"username\":  \"" << user->username << "\","
                  << "\"real_name\": \"" << user->real_name << "\","
                  << "\"role\": \"" << (user->role == UserRole::ADMIN ?  "admin" : "student") << "\","
                  << "\"email\": \"" << user->email << "\","
                  << "\"department\": \"" << user->department << "\""
                  << "}"
                  << "}";
    
    delete user;
    
    HttpResponse response(200, "OK");
    response.setJson(json_response.str());
    return response;
}

HttpResponse AuthRoutes::handleRegister(const HttpRequest& request) {
    if (request.method != "POST") {
        HttpResponse response(405, "Method Not Allowed");
        response.setJson(buildErrorResponse("Only POST method allowed"));
        return response;
    }
    
    // 解析注册数据
    std:: regex username_regex(R"("username"\s*:\s*"([^"]+)")");
    std::regex password_regex(R"("password"\s*:\s*"([^"]+)")");
    std::regex realname_regex(R"("real_name"\s*:\s*"([^"]+)")");
    std::regex email_regex(R"("email"\s*:\s*"([^"]+)")");
    std::regex department_regex(R"("department"\s*:\s*"([^"]+)")");
    std::regex role_regex(R"("role"\s*:\s*"([^"]+)")");
    
    std::smatch matches;
    
    if (!std::regex_search(request. body, matches, username_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing username"));
        return response;
    }
    std::string username = matches[1].str();
    
    if (!std::regex_search(request.body, matches, password_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing password"));
        return response;
    }
    std::string password = matches[1].str();
    
    if (!std::regex_search(request.body, matches, realname_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing real_name"));
        return response;
    }
    std::string real_name = matches[1].str();
    
    if (!std::regex_search(request.body, matches, email_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing email"));
        return response;
    }
    std::string email = matches[1].str();
    
    std::string department = "未指定";
    if (std::regex_search(request.body, matches, department_regex)) {
        department = matches[1].str();
    }
    
    UserRole role = UserRole::STUDENT;
    if (std:: regex_search(request.body, matches, role_regex)) {
        if (matches[1]. str() == "admin") {
            role = UserRole:: ADMIN;
        }
    }
    
    // 注册用户
    bool success = auth_manager->registerUser(username, password, real_name, email, department, role);
    
    if (success) {
        HttpResponse response(201, "Created");
        response.setJson("{\"success\": true, \"message\": \"User registered successfully\"}");
        return response;
    } else {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Registration failed"));
        return response;
    }
}

HttpResponse AuthRoutes::handleLogout(const HttpRequest& request) {
    HttpResponse response(200, "OK");
    response.setJson("{\"success\": true, \"message\":  \"Logged out successfully\"}");
    return response;
}

HttpResponse AuthRoutes::handleProfile(const HttpRequest& request) {
    // 这个需要认证中间件处理
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Profile endpoint requires authentication middleware"));
    return response;
}

HttpResponse AuthRoutes:: handleRefreshToken(const HttpRequest& request) {
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Token refresh not implemented yet"));
    return response;
}

// === AuthenticatedHttpServer 实现 ===

AuthenticatedHttpServer::AuthenticatedHttpServer(int server_port) 
    : port(server_port), running(false) {
    
    // 初始化所有管理器
    auth_manager = std:: make_unique<AuthManager>("data/auth. db");
    auth_middleware = std:: make_unique<AuthMiddleware>(auth_manager.get());
    auth_routes = std:: make_unique<AuthRoutes>(auth_manager.get());
    contact_manager = std:: make_unique<ContactManager>("data/contacts.db");
    activity_manager = std::make_unique<ActivityManager>("data/activities.db");
    
    std::vector<std::string> resources = {"报告厅", "体育馆", "实验室", "大礼堂", "会议室A", "会议室B"};
    conflict_detector = std:: make_unique<ConflictDetector>();
    conflict_detector->initialize(resources);
}

AuthenticatedHttpServer::~AuthenticatedHttpServer() {
    stop();
}

bool AuthenticatedHttpServer::initialize() {
    std::cout << "初始化认证HTTP服务器..." << std::endl;
    
    // 初始化认证管理器
    if (!auth_manager->initialize()) {
        std::cerr << "❌ 认证管理器初始化失败" << std::endl;
        return false;
    }
    
    // 初始化联系人管理器
    if (!contact_manager->initialize()) {
        std:: cerr << "❌ 联系人管理器初始化失败" << std::endl;
        return false;
    }
    
    // 初始化活动管理器
    if (!activity_manager->initialize()) {
        std::cerr << "❌ 活动管理器初始化失败" << std::endl;
        return false;
    }
    
    // 设置路由
    setupRoutes();
    
    std::cout << "✅ 认证HTTP服务器初始化成功" << std::endl;
    return true;
}

void AuthenticatedHttpServer::setupRoutes() {
    std::cout << "设置API路由..." << std::endl;
    
    // === 公开路由（无需认证）===
    registerPublicRoute("POST /api/auth/login", [this](const HttpRequest& req) {
        return auth_routes->handleLogin(req);
    });
    
    registerPublicRoute("POST /api/auth/register", [this](const HttpRequest& req) {
        return auth_routes->handleRegister(req);
    });
    
    registerPublicRoute("GET /", [this](const HttpRequest& req) {
        return serveStaticFile("frontend/v8.1.html");
    });
    
    registerPublicRoute("GET /index.html", [this](const HttpRequest& req) {
        return serveStaticFile("frontend/v8.1.html");
    });
    
    // === 受保护路由（需要认证）===
    registerProtectedRoute("GET /api/auth/profile", [this](const AuthenticatedRequest& req) {
        std::ostringstream json;
        json << "{"
             << "\"success\": true,"
             << "\"user\": {"
             << "\"id\":  " << req.current_user->id << ","
             << "\"username\":  \"" << req. current_user->username << "\","
             << "\"real_name\": \"" << req.current_user->real_name << "\","
             << "\"role\": \"" << (req.current_user->role == UserRole::ADMIN ? "admin" : "student") << "\","
             << "\"email\": \"" << req.current_user->email << "\","
             << "\"department\": \"" << req.current_user->department << "\""
             << "}"
             << "}";
        
        HttpResponse response;
        response.setJson(json.str());
        return response;
    });
    
    // 联系人API
    registerProtectedRoute("GET /api/contacts", [this](const AuthenticatedRequest& req) {
        return handleGetContacts(req);
    });
    
    registerProtectedRoute("POST /api/contacts", [this](const AuthenticatedRequest& req) {
        return handleCreateContact(req);
    });
    
    registerProtectedRoute("PUT /api/contacts", [this](const AuthenticatedRequest& req) {
        return handleUpdateContact(req);
    });
    
    registerProtectedRoute("DELETE /api/contacts", [this](const AuthenticatedRequest& req) {
        return handleDeleteContact(req);
    });
    
    registerProtectedRoute("GET /api/contacts/search", [this](const AuthenticatedRequest& req) {
        return handleSearchContacts(req);
    });
    
    // 活动API
    registerProtectedRoute("GET /api/activities", [this](const AuthenticatedRequest& req) {
        return handleGetActivities(req);
    });
    
    registerProtectedRoute("POST /api/activities", [this](const AuthenticatedRequest& req) {
        return handleCreateActivity(req);
    });
    
    registerProtectedRoute("PUT /api/activities", [this](const AuthenticatedRequest& req) {
        return handleUpdateActivity(req);
    });
    
    registerProtectedRoute("DELETE /api/activities", [this](const AuthenticatedRequest& req) {
        return handleDeleteActivity(req);
    });
    
    // 资源调度API（管理员专用）
    registerProtectedRoute("GET /api/schedule", [this](const AuthenticatedRequest& req) {
        if (! auth_middleware->authorize(req, UserRole::ADMIN)) {
            HttpResponse response(403, "Forbidden");
            response.setJson(buildErrorResponse("Admin access required"));
            return response;
        }
        return handleGetSchedule(req);
    });
    
    registerProtectedRoute("POST /api/schedule/check-conflict", [this](const AuthenticatedRequest& req) {
        if (!auth_middleware->authorize(req, UserRole:: ADMIN)) {
            HttpResponse response(403, "Forbidden");
            response.setJson(buildErrorResponse("Admin access required"));
            return response;
        }
        return handleCheckConflict(req);
    });
    
    std::cout << "✅ 路由设置完成" << std::endl;
}

bool AuthenticatedHttpServer::start() {
    if (running) {
        std::cout << "服务器已在运行" << std::endl;
        return true;
    }
    
    running = true;
    server_thread = std:: make_unique<std:: thread>(&AuthenticatedHttpServer::serverLoop, this);
    
    std::cout << "🚀 认证HTTP服务器启动成功" << std::endl;
    std::cout << "🌐 访问地址: http://localhost:" << port << std::endl;
    std::cout << "🔐 认证API: POST /api/auth/login" << std:: endl;
    std::cout << "👥 联系人API: GET /api/contacts (需要认证)" << std::endl;
    std::cout << "📅 活动API: GET /api/activities (需要认证)" << std::endl;
    std::cout << "⚡ 调度API: GET /api/schedule (管理员专用)" << std::endl;
    
    return true;
}

void AuthenticatedHttpServer::stop() {
    if (!running) return;
    
    running = false;
    if (server_thread && server_thread->joinable()) {
        server_thread->join();
    }
    
    std:: cout << "⏹️ 认证HTTP服务器已停止" << std::endl;
}

bool AuthenticatedHttpServer::isRunning() const {
    return running;
}

void AuthenticatedHttpServer::registerPublicRoute(const std::string& route, 
                                                 std::function<HttpResponse(const HttpRequest&)> handler) {
    public_routes[route] = handler;
}

void AuthenticatedHttpServer::registerProtectedRoute(const std::string& route, 
                                                    std:: function<HttpResponse(const AuthenticatedRequest&)> handler) {
    protected_routes[route] = handler;
}

void AuthenticatedHttpServer::serverLoop() {
    // 简化的服务器循环实现
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "❌ 无法创建服务器套接字" << std::endl;
        return;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address. sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "❌ 端口绑定失败" << std::endl;
        close(server_fd);
        return;
    }
    
    if (listen(server_fd, 10) < 0) {
        std:: cerr << "❌ 监听失败" << std::endl;
        close(server_fd);
        return;
    }
    
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            continue;
        }
        
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            // 解析HTTP请求
            HttpRequest request = parseHttpRequest(std::string(buffer, bytes_read));
            
            // 处理请求
            HttpResponse response = handleRequest(request);
            
            // 发送响应
            std::string response_str = buildHttpResponse(response);
            write(client_fd, response_str.c_str(), response_str.length());
        }
        
        close(client_fd);
    }
    
    close(server_fd);
}

HttpResponse AuthenticatedHttpServer::handleRequest(const HttpRequest& request) {
    // 处理OPTIONS请求（CORS预检）
    if (request.method == "OPTIONS") {
        HttpResponse response(200, "OK");
        response.setCORS();
        return response;
    }
    
    std::string route_key = request.method + " " + request.path;
    
    // 检查公开路由
    auto public_it = public_routes. find(route_key);
    if (public_it != public_routes.end()) {
        return public_it->second(request);
    }
    
    // 检查受保护路由
    auto protected_it = protected_routes.find(route_key);
    if (protected_it != protected_routes.end()) {
        // 创建认证请求
        AuthenticatedRequest auth_request(request);
        
        // 验证认证
        if (! auth_middleware->authenticate(auth_request)) {
            HttpResponse response(401, "Unauthorized");
            response.setJson(buildErrorResponse("Authentication required"));
            return response;
        }
        
        return protected_it->second(auth_request);
    }
    
    // 尝试静态文件服务
    if (request.method == "GET") {
        if (request.path == "/" || request.path == "/index.html") {
            return serveStaticFile("frontend/index-modern.html");
        }
        if (request.path.find("/css/") == 0) {
            return serveStaticFile("frontend" + request.path);
        }
        if (request.path.find("/js/") == 0) {
            return serveStaticFile("frontend" + request.path);
        }
    }
    
    // 404 Not Found
    HttpResponse response(404, "Not Found");
    response.setJson(buildErrorResponse("Endpoint not found"));
    return response;
}

// API处理器实现

HttpResponse AuthenticatedHttpServer::handleGetContacts(const AuthenticatedRequest& request) {
    auto contacts = contact_manager->getAllContacts();
    
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < contacts.size(); ++i) {
        json << "{"
             << "\"id\": " << contacts[i].id << ","
             << "\"name\": \"" << contacts[i].name << "\","
             << "\"phone\": \"" << contacts[i].phone << "\","
             << "\"email\": \"" << contacts[i].email << "\"";
        if (!contacts[i].department.empty()) {
            json << ",\"department\": \"" << contacts[i].department << "\"";
        }
        if (!contacts[i].student_id.empty()) {
            json << ",\"student_id\": \"" << contacts[i].student_id << "\"";
        }
        json << "}";
        if (i < contacts. size() - 1) json << ",";
    }
    json << "]";
    
    HttpResponse response;
    response.setJson(json.str());
    return response;
}

HttpResponse AuthenticatedHttpServer::handleGetActivities(const AuthenticatedRequest& request) {
    auto activities = activity_manager->getAllActivities();
    
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < activities.size(); ++i) {
        json << "{"
             << "\"id\": " << activities[i].id << ","
             << "\"name\": \"" << activities[i].name << "\","
             << "\"location\": \"" << activities[i].location << "\","
             << "\"start_time\": \"" << activities[i].start_time << "\","
             << "\"end_time\": \"" << activities[i].end_time << "\""
             << "}";
        if (i < activities.size() - 1) json << ",";
    }
    json << "]";
    
    HttpResponse response;
    response.setJson(json.str());
    return response;
}

HttpResponse AuthenticatedHttpServer::handleCreateContact(const AuthenticatedRequest& request) {
    if (request.method != "POST") {
        HttpResponse response(405, "Method Not Allowed");
        response.setJson(buildErrorResponse("Only POST method allowed"));
        return response;
    }
    
    // 解析JSON请求体
    std::regex name_regex(R"("name"\s*:\s*"([^"]+)")");
    std::regex phone_regex(R"("phone"\s*:\s*"([^"]+)")");
    std::regex email_regex(R"("email"\s*:\s*"([^"]+)")");
    std::regex department_regex(R"("department"\s*:\s*"([^"]+)")");
    
    std::smatch matches;
    std::string name, phone, email, department = "";
    
    if (!std::regex_search(request.body, matches, name_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing name field"));
        return response;
    }
    name = matches[1].str();
    
    if (!std::regex_search(request.body, matches, phone_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing phone field"));
        return response;
    }
    phone = matches[1].str();
    
    if (!std::regex_search(request.body, matches, email_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing email field"));
        return response;
    }
    email = matches[1].str();
    
    if (std::regex_search(request.body, matches, department_regex)) {
        department = matches[1].str();
    }
    
    // 检查重复
    if (contact_manager->hasDuplicateEmail(email)) {
        HttpResponse response(409, "Conflict");
        response.setJson(buildErrorResponse("Contact with this email already exists", 409));
        return response;
    }
    
    // 创建联系人
    Contact contact(0, name, "", phone, email, department);
    if (contact_manager->addContact(contact)) {
        HttpResponse response(201, "Created");
        response.setJson("{\"success\": true, \"message\": \"Contact created successfully\"}");
        return response;
    } else {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Failed to create contact"));
        return response;
    }
}

HttpResponse AuthenticatedHttpServer::handleCreateActivity(const AuthenticatedRequest& request) {
    if (request.method != "POST") {
        HttpResponse response(405, "Method Not Allowed");
        response.setJson(buildErrorResponse("Only POST method allowed"));
        return response;
    }
    
    // 解析JSON请求体
    std::regex name_regex(R"("name"\s*:\s*"([^"]+)")");
    std::regex location_regex(R"("location"\s*:\s*"([^"]+)")");
    std::regex start_time_regex(R"("start_time"\s*:\s*"([^"]+)")");
    std::regex end_time_regex(R"("end_time"\s*:\s*"([^"]+)")");
    std::regex max_participants_regex(R"("max_participants"\s*:\s*(\d+))");
    
    std::smatch matches;
    std::string name, location, start_time, end_time;
    int max_participants = 0;
    
    if (!std::regex_search(request.body, matches, name_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing name field"));
        return response;
    }
    name = matches[1].str();
    
    if (!std::regex_search(request.body, matches, location_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing location field"));
        return response;
    }
    location = matches[1].str();
    
    if (!std::regex_search(request.body, matches, start_time_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing start_time field"));
        return response;
    }
    start_time = matches[1].str();
    
    if (!std::regex_search(request.body, matches, end_time_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing end_time field"));
        return response;
    }
    end_time = matches[1].str();
    
    if (std::regex_search(request.body, matches, max_participants_regex)) {
        max_participants = std::stoi(matches[1].str());
    }
    
    // 创建活动对象
    Activity activity(0, name, location, start_time, end_time);
    
    // 检查冲突
    if (activity_manager->hasTimeConflict(activity)) {
        auto conflicts = activity_manager->findConflictingActivities(activity);
        std::ostringstream json;
        json << "{"
             << "\"success\": false,"
             << "\"error\": \"Time conflict detected for the specified location\","
             << "\"code\": 409,"
             << "\"conflicts\": [";
        for (size_t i = 0; i < conflicts.size(); ++i) {
            json << "{"
                 << "\"id\": " << conflicts[i].id << ","
                 << "\"name\": \"" << conflicts[i].name << "\""
                 << "}";
            if (i < conflicts.size() - 1) json << ",";
        }
        json << "]}";
        
        HttpResponse response(409, "Conflict");
        response.setJson(json.str());
        return response;
    }
    
    // 添加活动
    if (activity_manager->addActivity(activity)) {
        HttpResponse response(201, "Created");
        response.setJson("{\"success\": true, \"message\": \"Activity created successfully\"}");
        return response;
    } else {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Failed to create activity"));
        return response;
    }
}

HttpResponse AuthenticatedHttpServer::handleCheckConflict(const AuthenticatedRequest& request) {
    if (request.method != "POST") {
        HttpResponse response(405, "Method Not Allowed");
        response.setJson(buildErrorResponse("Only POST method allowed"));
        return response;
    }
    
    // 解析JSON请求体
    std::regex location_regex(R"("location"\s*:\s*"([^"]+)")");
    std::regex start_time_regex(R"("start_time"\s*:\s*"([^"]+)")");
    std::regex end_time_regex(R"("end_time"\s*:\s*"([^"]+)")");
    
    std::smatch matches;
    std::string location, start_time, end_time;
    
    if (!std::regex_search(request.body, matches, location_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing location field"));
        return response;
    }
    location = matches[1].str();
    
    if (!std::regex_search(request.body, matches, start_time_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing start_time field"));
        return response;
    }
    start_time = matches[1].str();
    
    if (!std::regex_search(request.body, matches, end_time_regex)) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing end_time field"));
        return response;
    }
    end_time = matches[1].str();
    
    // 检查冲突
    Activity test_activity(0, "", location, start_time, end_time);
    bool has_conflict = activity_manager->hasTimeConflict(test_activity);
    
    std::ostringstream json;
    json << "{"
         << "\"success\": true,"
         << "\"has_conflict\": " << (has_conflict ? "true" : "false") << ",";
    
    if (has_conflict) {
        auto conflicts = activity_manager->findConflictingActivities(test_activity);
        json << "\"conflicts\": [";
        for (size_t i = 0; i < conflicts.size(); ++i) {
            json << "{"
                 << "\"activity_id\": " << conflicts[i].id << ","
                 << "\"activity_name\": \"" << conflicts[i].name << "\","
                 << "\"start_time\": \"" << conflicts[i].start_time << "\","
                 << "\"end_time\": \"" << conflicts[i].end_time << "\""
                 << "}";
            if (i < conflicts.size() - 1) json << ",";
        }
        json << "]";
    } else {
        json << "\"message\": \"No conflict detected\"";
    }
    
    json << "}";
    
    HttpResponse response;
    response.setJson(json.str());
    return response;
}

HttpResponse AuthenticatedHttpServer::handleGetSchedule(const AuthenticatedRequest& request) {
    // 获取资源调度信息
    auto resources = conflict_detector->getAvailableResources();
    
    std::ostringstream json;
    json << "{"
         << "\"success\": true,"
         << "\"resources\": [";
    
    for (size_t i = 0; i < resources. size(); ++i) {
        json << "\"" << resources[i] << "\"";
        if (i < resources. size() - 1) json << ",";
    }
    
    json << "],"
         << "\"total_reservations\": " << conflict_detector->getTotalReservations()
         << "}";
    
    HttpResponse response;
    response.setJson(json.str());
    return response;
}

HttpResponse AuthenticatedHttpServer::handleUpdateContact(const AuthenticatedRequest& request) {
    // 从路径中提取ID（简化实现，实际应该从路径参数中提取）
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Update contact not fully implemented yet"));
    return response;
}

HttpResponse AuthenticatedHttpServer::handleDeleteContact(const AuthenticatedRequest& request) {
    // 从查询参数或路径中提取ID
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Delete contact not fully implemented yet"));
    return response;
}

HttpResponse AuthenticatedHttpServer::handleSearchContacts(const AuthenticatedRequest& request) {
    // 从查询参数中获取搜索关键词
    std::string query = request.query_string;
    size_t q_pos = query.find("q=");
    if (q_pos == std::string::npos) {
        HttpResponse response(400, "Bad Request");
        response.setJson(buildErrorResponse("Missing query parameter 'q'"));
        return response;
    }
    
    std::string search_term = query.substr(q_pos + 2);
    // URL解码（简化实现）
    size_t amp_pos = search_term.find('&');
    if (amp_pos != std::string::npos) {
        search_term = search_term.substr(0, amp_pos);
    }
    
    // 使用Trie树搜索
    auto contacts = contact_manager->searchByName(search_term);
    
    std::ostringstream json;
    json << "{"
         << "\"success\": true,"
         << "\"data\": [";
    for (size_t i = 0; i < contacts.size(); ++i) {
        json << "{"
             << "\"id\": " << contacts[i].id << ","
             << "\"name\": \"" << contacts[i].name << "\","
             << "\"phone\": \"" << contacts[i].phone << "\","
             << "\"email\": \"" << contacts[i].email << "\""
             << "}";
        if (i < contacts.size() - 1) json << ",";
    }
    json << "],"
         << "\"total\": " << contacts.size()
         << "}";
    
    HttpResponse response;
    response.setJson(json.str());
    return response;
}

HttpResponse AuthenticatedHttpServer::handleUpdateActivity(const AuthenticatedRequest& request) {
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Update activity not fully implemented yet"));
    return response;
}

HttpResponse AuthenticatedHttpServer::handleDeleteActivity(const AuthenticatedRequest& request) {
    // 从查询参数或路径中提取ID
    HttpResponse response(501, "Not Implemented");
    response.setJson(buildErrorResponse("Delete activity not fully implemented yet"));
    return response;
}

// 工具方法

std::string AuthenticatedHttpServer::buildErrorResponse(const std:: string& error, int code) {
    std::ostringstream json;
    json << "{"
         << "\"success\": false,"
         << "\"error\": \"" << error << "\","
         << "\"code\": " << code
         << "}";
    return json.str();
}

HttpResponse AuthenticatedHttpServer::serveStaticFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.good()) {
        HttpResponse response(404, "Not Found");
        response.setJson(buildErrorResponse("File not found"));
        return response;
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    
    HttpResponse response;
    response.body = content.str();
    
    // 设置Content-Type
    if (path.find(". html") != std::string::npos) {
        response. headers["Content-Type"] = "text/html";
    } else if (path.find(".css") != std::string::npos) {
        response.headers["Content-Type"] = "text/css";
    } else if (path.find(".js") != std::string:: npos) {
        response.headers["Content-Type"] = "application/javascript";
    }
    
    return response;
}

// 需要实现的辅助函数
HttpRequest AuthenticatedHttpServer::parseHttpRequest(const std::string& raw_request) {
    // 简化的HTTP请求解析
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;
    
    // 解析请求行
    if (std::getline(stream, line)) {
        if (line.back() == '\r') line.pop_back();
        std:: istringstream request_line(line);
        std::string path_with_query, http_version;
        request_line >> request.method >> path_with_query >> http_version;
        
        size_t query_pos = path_with_query.find('?');
        if (query_pos != std::string::npos) {
            request.path = path_with_query.substr(0, query_pos);
            request.query_string = path_with_query.substr(query_pos + 1);
        } else {
            request.path = path_with_query;
        }
    }
    
    // 解析请求头
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        if (line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string:: npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // 去除前导空格
            value.erase(0, value.find_first_not_of(" \t"));
            if (! value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            request.headers[key] = value;
        }
    }
    
    // 读取请求体（如果有Content-Length头）
    std::ostringstream body_stream;
    std::string body_line;
    while (std::getline(stream, body_line)) {
        body_stream << body_line;
        if (!stream.eof()) body_stream << "\n";
    }
    request. body = body_stream.str();
    
    return request;
}

std::string AuthenticatedHttpServer::buildHttpResponse(const HttpResponse& response) {
    std::ostringstream oss;
    
    oss << "HTTP/1.1 " << response.status_code << " " << response.status_text << "\r\n";
    
    // 设置CORS头
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    
    for (const auto& header : response.headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body. length() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
}

// 简单的路由模式匹配（支持 /api/contacts/{id} 这样的模式）
bool AuthenticatedHttpServer::matchRoutePattern(const std::string& pattern, const std::string& path) {
    // 简化实现：检查路径前缀是否匹配
    // 例如：pattern = "GET /api/contacts", path = "GET /api/contacts/1"
    if (path.find(pattern) == 0) {
        // 检查是否是完全匹配或者是路径参数的扩展
        if (path.length() == pattern.length() || path[pattern.length()] == '/') {
            return true;
        }
    }
    return false;
}