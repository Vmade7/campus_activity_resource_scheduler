#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include "data_manager.h"
#include "segment_tree.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

class ActivityManager {
private: 
    std::unique_ptr<DataManager> data_manager;           // 数据管理器
    std::unique_ptr<SegmentTree> conflict_detector;     // 冲突检测器
    std::map<std::string, std::vector<int>> location_activities;  // 地点-活动映射
    
    bool conflict_detection_enabled;                     // 是否启用冲突检测

public:
    ActivityManager(const std::string& db_path = "data/database.db",
                   const std::string& backup_dir = "data/");
    ~ActivityManager();
    
    // 禁用拷贝
    ActivityManager(const ActivityManager&) = delete;
    ActivityManager& operator=(const ActivityManager&) = delete;
    
    // 初始化
    bool initialize();
    bool isReady() const;
    
    // 基本活动操作
    bool addActivity(const std::string& name, const std::string& location, 
                    const std:: string& start_time, const std::string& end_time);
    bool addActivity(const Activity& activity);
    bool removeActivity(int id);
    bool updateActivity(const Activity& activity);
    
    // 高级查询功能
    std::vector<Activity> findByLocation(const std::string& location);     // 按地点查找
    std::vector<Activity> findByTimeRange(const std::string& start, const std::string& end);  // 按时间范围
    Activity* findById(int id);                                           // 按ID查找
    std::vector<Activity> getAllActivities();                             // 获取所有活动
    
    // 冲突检测
    bool hasTimeConflict(const Activity& activity);                       // 检查时间冲突
    std::vector<Activity> findConflictingActivities(const Activity& activity);  // 找出冲突的活动
    void enableConflictDetection(bool enable = true);                     // 启用/禁用冲突检测
    
    // 资源调度
    std::vector<std::string> getAvailableLocations(const std::string& start_time, const std::string& end_time);
    std::vector<Activity> getUpcomingActivities(int days = 7);           // 获取即将到来的活动
    
    // 数据分析  
    int getTotalCount();
    std::map<std::string, int> getLocationUsageStats();                   // 地点使用统计
    std::vector<Activity> getMostPopularActivities(int limit = 5);       // 热门活动
    
    // 批量操作
    bool importActivities(const std::vector<Activity>& activities);       // 批量导入
    bool exportActivities(const std::string& filename);                   // 导出到文件
    
    // 工具方法
    void printScheduleSummary();                                          // 打印日程摘要
    void printConflictReport();                                          // 打印冲突报告
    
private:
    // 内部辅助方法
    void updateLocationIndex(const Activity& activity);                   // 更新地点索引
    void removeFromLocationIndex(const Activity& activity);               // 从地点索引移除
    bool validateActivity(const Activity& activity);                      // 活动验证
    Activity createActivity(const std::string& name, const std::string& location,
                           const std::string& start_time, const std::string& end_time);
    
    // 时间处理辅助方法
    int parseTimeToMinutes(const std::string& time_str);                 // 时间字符串转分钟
    std::string minutesToTimeString(int minutes);                        // 分钟转时间字符串
    bool isTimeInRange(const std::string& time, const std::string& start, const std::string& end);
};

#endif // ACTIVITY_MANAGER_H