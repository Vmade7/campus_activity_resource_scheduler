#include "../include/segment_tree.h"
#include <iostream>

void testBasicSegmentTree() {
    std::cout << "=== 测试基础线段树功能 ===\n";
    
    SegmentTree st(24);  // 24小时时间范围
    
    // 测试添加时间段
    std::cout << "1. 添加活动时间段 [9, 11] (9-11点)\n";
    st.addInterval(9, 11);
    
    // 测试冲突检查
    std::cout << "2. 检查冲突:\n";
    std::cout << "   [8, 10] 冲突: " << (st.isConflict(8, 10) ? "是" : "否") << "\n";
    std:: cout << "   [12, 14] 冲突: " << (st.isConflict(12, 14) ? "是" : "否") << "\n";
    std::cout << "   [10, 11] 冲突: " << (st.isConflict(10, 11) ? "是" : "否") << "\n";
    
    // 添加更多时间段
    std::cout << "\n3. 添加更多活动 [14, 16]\n";
    st.addInterval(14, 16);
    
    // 查询占用情况
    std:: cout << "4. 查询占用情况:\n";
    std:: cout << "   [9, 11] 占用次数: " << st.queryOccupancy(9, 11) << "\n";
    std:: cout << "   [0, 23] 最大占用:  " << st.queryOccupancy(0, 23) << "\n";
    
    std::cout << "\n";
}

void testVenueConflictDetector() {
    std::cout << "=== 测试场地冲突检测器 ===\n";
    
    VenueConflictDetector detector(24 * 60);  // 24小时，按分钟计算
    
    // 添加活动
    std::cout << "1. 添加活动:\n";
    bool result1 = detector.addActivity("A101", 540, 660, 1);  // 9:00-11:00
    bool result2 = detector.addActivity("A101", 720, 840, 2);  // 12:00-14:00
    bool result3 = detector.addActivity("A101", 600, 720, 3);  // 10:00-12:00 (冲突)
    
    std::cout << "   A101 9:00-11:00: " << (result1 ? "成功" : "失败") << "\n";
    std::cout << "   A101 12:00-14:00: " << (result2 ? "成功" : "失败") << "\n";
    std:: cout << "   A101 10:00-12:00: " << (result3 ? "成功" : "失败") << " (预期失败)\n";
    
    // 不同场地测试
    std:: cout << "\n2. 不同场地测试:\n";
    bool result4 = detector.addActivity("B201", 600, 720, 4);  // B201 10:00-12:00
    std::cout << "   B201 10:00-12:00: " << (result4 ? "成功" : "失败") << "\n";
    
    // 查找可用时间段
    std::cout << "\n3. 查找A101的可用2小时时间段:\n";
    auto slots = detector.findAvailableSlots("A101", 120);  // 2小时 = 120分钟
    for (const auto& slot : slots) {
        int startHour = slot.first / 60;
        int startMin = slot.first % 60;
        int endHour = slot.second / 60;
        int endMin = slot.second % 60;
        std:: cout << "   " << startHour << ":" << (startMin < 10 ? "0" : "") << startMin 
                  << " - " << endHour << ":" << (endMin < 10 ? "0" : "") << endMin << "\n";
    }
    
    std::cout << "\n";
}

int main() {
    testBasicSegmentTree();
    testVenueConflictDetector();
    
    std::cout << "=== 线段树测试完成 ===\n";
    return 0;
}