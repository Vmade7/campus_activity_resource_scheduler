#include "../include/http_server_auth.h"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <chrono>

// 全局服务器指针，用于信号处理
std::unique_ptr<AuthenticatedHttpServer> g_server = nullptr;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\n\n收到信号 " << signal << "，正在优雅关闭服务器..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    std::exit(0);
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 解析命令行参数
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "  校园活动资源调度系统" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // 创建并初始化服务器
    g_server.reset(new AuthenticatedHttpServer(port));
    
    if (!g_server->initialize()) {
        std::cerr << "服务器初始化失败！" << std::endl;
        return 1;
    }
    
    // 启动服务器
    if (!g_server->start()) {
        std::cerr << "服务器启动失败！" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "服务器运行中..." << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    std::cout << std::endl;
    
    // 保持主线程运行
    while (g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}

