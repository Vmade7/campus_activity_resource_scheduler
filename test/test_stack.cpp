#include "../include/stack.h"
#include <iostream>
#include <string>

void testBasicOperations() {
    std::cout << "=== 测试1: 基础操作 ===\n";
    
    Stack<int> stack;
    
    // 入栈
    std::cout << "1. 入栈测试:\n";
    stack.push(10);
    stack.push(20);
    stack.push(30);
    stack.print();  // 应该显示: 30 20 10
    
    // 获取栈顶并出栈
    std::cout << "2. 获取栈顶和出栈:\n";
    std::cout << "   栈顶元素: " << stack.top() << "\n"; // 应该是30
    stack.pop();  // 移除30
    stack.print();  // 应该显示: 20 10
    
    // 测试空栈异常
    std::cout << "3. 测试空栈:\n";
    stack.pop();  // 移除20
    stack.pop();  // 移除10
    stack.print();  // 应该显示: 栈内容为空
    
    try {
        stack.pop();  // 应该抛出异常
    } catch (const std::underflow_error& e) {
        std::cout << "   异常捕获: " << e.what() << "\n";
    }
}

void testAdvancedOperations() {
    std::cout << "\n=== 测试2: 高级操作 ===\n";
    
    Stack<std::string> stack;
    stack.push("第一条记录");
    stack.push("第二条记录");
    stack.push("第三条记录");
    
    std::cout << "1. 全部打印:\n";
    stack.print();  // 打印所有记录
    
    std::cout << "2. 清空栈:\n";
    stack.clear();
    stack.print();  // 应该显示为空
}

void testUndoRedo() {
    std::cout << "\n=== 测试3: 撤销操作（Undo/Redo） ===\n";
    
    // 历史记录和撤销栈
    Stack<std::string> historyStack;
    Stack<std::string> redoStack;
    
    historyStack.push("操作1");
    historyStack.push("操作2");
    historyStack.push("操作3");
    
    std::cout << "1. 当前历史记录:\n";
    historyStack.print();
    
    // 撤销（Undo）
    std::cout << "2. 撤销操作:\n";
    redoStack.push(historyStack.top());
    historyStack.pop();
    historyStack.print();  // 应该是: 操作2 操作1
    
    // 重做（Redo）
    std::cout << "3. 重做操作:\n";
    historyStack.push(redoStack.top());
    redoStack.pop();
    historyStack.print();  // 应该是: 操作3 操作2 操作1
}

int main() {
    testBasicOperations();
    testAdvancedOperations();
    testUndoRedo();
    
    std::cout << "\n=== 栈测试完成 ===\n";
    return 0;
}