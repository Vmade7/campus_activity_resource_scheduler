#include "../include/activity_manager.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>

ActivityManager::ActivityManager(const std::string& db_path, const std::string& backup_dir)
    : conflict_detection_enabled(true) {
    
    data_manager.reset(new DataManager(db_path, backup_dir, 100));
    conflict_detector.reset(new SegmentTree(1440)); // 一天1440分钟
}

ActivityManager::~ActivityManager() = default;

bool ActivityManager::initialize() {
    std::cout << "初始化活动管理器..." << std::endl;
    
    // 初始化数据管理器
    if (!data_manager->initialize()) {
        std::cerr << "数据管理器初始化失败" << std::endl;
        return false;
    }
    
    // 构建地点索引
    auto activities = data_manager->getAllActivities();
    for (const auto& activity : activities) {
        updateLocationIndex(activity);
    }
    
    std::cout << "活动管理器初始化成功，加载了 " << activities.size() << " 个活动" << std::endl;
    return true;
}

bool ActivityManager::isReady() const {
    return data_manager && data_manager->isReady();
}

// 基本操作

bool ActivityManager::addActivity(const std::string& name, const std::string& location,
                                 const std::string& start_time, const std::string& end_time) {
    Activity activity = createActivity(name, location, start_time, end_time);
    return addActivity(activity);
}

bool ActivityManager::addActivity(const Activity& activity) {
    if (!isReady()) {
        std::cerr << "活动管理器未就绪" << std::endl;
        return false;
    }
    
    if (! validateActivity(activity)) {
        return false;
    }
    
    // 冲突检测
    if (conflict_detection_enabled && hasTimeConflict(activity)) {
        auto conflicts = findConflictingActivities(activity);
        std::cerr << "时间冲突，与以下活动冲突:" << std::endl;
        for (const auto& conflict : conflicts) {
            std::cerr << "  - " << conflict.name << " @ " << conflict.location 
                     << " (" << conflict.start_time << " - " << conflict.end_time << ")" << std::endl;
        }
        return false;
    }
    
    // 添加到数据存储
    if (! data_manager->addActivity(activity)) {
        return false;
    }
    
    // 获取插入后的活动
    auto activities = data_manager->getAllActivities();
    if (! activities.empty()) {
        const Activity& newActivity = activities. back();
        updateLocationIndex(newActivity);
        std::cout << "活动已添加: " << newActivity.name << " @ " << newActivity. location 
                 << " (ID: " << newActivity.id << ")" << std::endl;
    }
    
    return true;
}

bool ActivityManager:: removeActivity(int id) {
    if (!isReady()) return false;
    
    // 先获取活动信息
    Activity* activity = data_manager->getActivity(id);
    if (!activity) {
        std::cerr << "活动不存在: ID=" << id << std::endl;
        return false;
    }
    
    Activity activityCopy = *activity;
    delete activity;
    
    // 从数据存储中删除
    if (!data_manager->deleteActivity(id)) {
        return false;
    }
    
    // 从索引中移除
    removeFromLocationIndex(activityCopy);
    
    std::cout << "活动已删除: " << activityCopy.name << std::endl;
    return true;
}

bool ActivityManager::updateActivity(const Activity& activity) {
    if (!isReady() || !validateActivity(activity)) return false;
    
    // 更新数据存储
    if (!data_manager->updateActivity(activity)) {
        return false;
    }
    
    // 更新索引
    updateLocationIndex(activity);
    
    std::cout << "活动已更新: " << activity.name << std::endl;
    return true;
}

// 高级查询

std::vector<Activity> ActivityManager::findByLocation(const std::string& location) {
    std::vector<Activity> result;
    if (!isReady()) return result;
    
    auto activities = data_manager->getAllActivities();
    for (const auto& activity :  activities) {
        if (activity.location. find(location) != std::string::npos) {
            result.push_back(activity);
        }
    }
    
    std::cout << "在地点 '" << location << "' 找到 " << result.size() << " 个活动" << std:: endl;
    return result;
}

std::vector<Activity> ActivityManager::findByTimeRange(const std::string& start, const std::string& end) {
    std::vector<Activity> result;
    if (!isReady()) return result;
    
    auto activities = data_manager->getAllActivities();
    for (const auto& activity : activities) {
        if (isTimeInRange(activity.start_time, start, end) || 
            isTimeInRange(activity.end_time, start, end)) {
            result. push_back(activity);
        }
    }
    
    std::cout << "在时间段 " << start << " - " << end << " 找到 " << result.size() << " 个活动" << std::endl;
    return result;
}

Activity* ActivityManager:: findById(int id) {
    if (!isReady()) return nullptr;
    return data_manager->getActivity(id);
}

std::vector<Activity> ActivityManager::getAllActivities() {
    if (!isReady()) return {};
    return data_manager->getAllActivities();
}

// 冲突检测

bool ActivityManager::hasTimeConflict(const Activity& activity) {
    if (!conflict_detection_enabled) return false;
    
    // 简化的冲突检测：检查相同地点的时间重叠
    auto locationActivities = findByLocation(activity.location);
    
    for (const auto& existing : locationActivities) {
        // 检查时间重叠
        if (!(activity.end_time <= existing.start_time || activity.start_time >= existing.end_time)) {
            return true;
        }
    }
    
    return false;
}

std::vector<Activity> ActivityManager::findConflictingActivities(const Activity& activity) {
    std::vector<Activity> conflicts;
    
    auto locationActivities = findByLocation(activity.location);
    
    for (const auto& existing :  locationActivities) {
        if (!(activity.end_time <= existing.start_time || activity. start_time >= existing.end_time)) {
            conflicts.push_back(existing);
        }
    }
    
    return conflicts;
}

void ActivityManager::enableConflictDetection(bool enable) {
    conflict_detection_enabled = enable;
    std::cout << "冲突检测已" << (enable ? "启用" :  "禁用") << std::endl;
}

// 资源调度

std::vector<std::string> ActivityManager:: getAvailableLocations(const std::string& start_time, 
                                                               const std:: string& end_time) {
    std::vector<std::string> availableLocations;
    
    // 预定义一些地点（实际项目中应该从配置或数据库读取）
    std::vector<std::string> allLocations = {
        "会议室A", "会议室B", "培训室1", "培训室2", "大礼堂", "小礼堂", "展览厅"
    };
    
    for (const auto& location : allLocations) {
        bool available = true;
        auto locationActivities = findByLocation(location);
        
        for (const auto& activity : locationActivities) {
            if (!(end_time <= activity.start_time || start_time >= activity. end_time)) {
                available = false;
                break;
            }
        }
        
        if (available) {
            availableLocations.push_back(location);
        }
    }
    
    std::cout << "时间段 " << start_time << " - " << end_time 
             << " 可用地点: " << availableLocations.size() << " 个" << std::endl;
    return availableLocations;
}

std::vector<Activity> ActivityManager::getUpcomingActivities(int days) {
    std::vector<Activity> upcoming;
    
    // 简化实现：返回所有活动（实际项目中需要日期比较）
    auto activities = getAllActivities();
    std::cout << "📅 即将到来的活动: " << activities.size() << " 个" << std:: endl;
    return activities;
}

// 数据分析

int ActivityManager:: getTotalCount() {
    if (!isReady()) return 0;
    return data_manager->getActivityCount();
}

std::map<std::string, int> ActivityManager::getLocationUsageStats() {
    std::map<std::string, int> stats;
    
    auto activities = getAllActivities();
    for (const auto& activity : activities) {
        stats[activity. location]++;
    }
    
    return stats;
}

std:: vector<Activity> ActivityManager::getMostPopularActivities(int limit) {
    auto activities = getAllActivities();
    
    // 简化实现：按名称排序返回前几个
    std::sort(activities. begin(), activities.end(),
              [](const Activity& a, const Activity& b) {
                  return a.name < b.name;
              });
    
    if (activities.size() > static_cast<size_t>(limit)) {
        activities.resize(limit);
    }
    
    return activities;
}

// 批量操作

bool ActivityManager:: importActivities(const std::vector<Activity>& activities) {
    if (!isReady()) return false;
    
    int successCount = 0;
    int errorCount = 0;
    
    // 临时禁用冲突检测
    bool originalConflictSetting = conflict_detection_enabled;
    enableConflictDetection(false);
    
    for (const auto& activity : activities) {
        if (addActivity(activity)) {
            successCount++;
        } else {
            errorCount++;
        }
    }
    
    // 恢复冲突检测设置
    enableConflictDetection(originalConflictSetting);
    
    std::cout << "活动导入完成: 成功 " << successCount << " 个, 失败 " << errorCount << " 个" << std:: endl;
    return errorCount == 0;
}

bool ActivityManager::exportActivities(const std::string& filename) {
    if (!isReady()) return false;
    return data_manager->backupAllData();
}

// 工具方法

void ActivityManager::printScheduleSummary() {
    if (!isReady()) {
        std::cout << "活动管理器未就绪" << std::endl;
        return;
    }
    
    std::cout << "=== 活动日程摘要 ===" << std::endl;
    std:: cout << "总活动数: " << getTotalCount() << std::endl;
    
    auto locationStats = getLocationUsageStats();
    std::cout << "地点使用情况:" << std::endl;
    for (const auto& stat :  locationStats) {
        std::cout << "  " << stat. first << ": " << stat. second << " 个活动" << std::endl;
    }
}

void ActivityManager::printConflictReport() {
    std::cout << "=== 冲突检测报告 ===" << std::endl;
    std::cout << "冲突检测:  " << (conflict_detection_enabled ? "启用" : "禁用") << std::endl;
    
    if (conflict_detection_enabled) {
        // 简化实现：检查所有活动的潜在冲突
        auto activities = getAllActivities();
        int conflictCount = 0;
        
        for (const auto& activity : activities) {
            auto conflicts = findConflictingActivities(activity);
            if (!conflicts.empty()) {
                conflictCount++;
            }
        }
        
        std::cout << "发现冲突活动: " << conflictCount << " 个" << std::endl;
    }
}

// 私有辅助方法

void ActivityManager::updateLocationIndex(const Activity& activity) {
    if (activity.id <= 0) return;
    location_activities[activity.location].push_back(activity. id);
}

void ActivityManager::removeFromLocationIndex(const Activity& activity) {
    auto& ids = location_activities[activity.location];
    ids.erase(std::remove(ids.begin(), ids.end(), activity.id), ids.end());
}

bool ActivityManager::validateActivity(const Activity& activity) {
    if (activity.name.empty()) {
        std::cerr << "活动名称不能为空" << std::endl;
        return false;
    }
    
    if (activity.location.empty()) {
        std::cerr << "活动地点不能为空" << std::endl;
        return false;
    }
    
    if (activity. start_time.empty() || activity.end_time.empty()) {
        std::cerr << "活动时间不能为空" << std::endl;
        return false;
    }
    
    if (activity.start_time >= activity.end_time) {
        std::cerr << "开始时间必须早于结束时间" << std::endl;
        return false;
    }
    
    return true;
}

Activity ActivityManager::createActivity(const std::string& name, const std:: string& location,
                                        const std::string& start_time, const std::string& end_time) {
    return Activity(0, name, location, start_time, end_time);
}

int ActivityManager::parseTimeToMinutes(const std::string& time_str) {
    // 简化实现：假设格式为 "HH:MM"
    std::istringstream ss(time_str);
    std::string hour_str, minute_str;
    std::getline(ss, hour_str, ':');
    std::getline(ss, minute_str);
    
    int hours = std::stoi(hour_str);
    int minutes = std::stoi(minute_str);
    
    return hours * 60 + minutes;
}

std::string ActivityManager::minutesToTimeString(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << mins;
    
    return ss.str();
}

bool ActivityManager::isTimeInRange(const std::string& time, const std::string& start, const std::string& end) {
    return time >= start && time <= end;
}