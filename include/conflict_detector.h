#ifndef CONFLICT_DETECTOR_H
#define CONFLICT_DETECTOR_H

#include "segment_tree.h"
#include "sqlite_manager.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>

// 时间段结构
struct TimeSlot {
    std::string start_time;
    std::string end_time;
    int start_minutes;      // 转换为分钟数便于计算
    int end_minutes;
    
    TimeSlot() : start_minutes(0), end_minutes(0) {}  // 默认构造函数
    TimeSlot(const std::string& start, const std::string& end);
    bool overlaps(const TimeSlot& other) const;
    std::string toString() const;
};

// 资源预约结构
struct Reservation {
    int id;
    std:: string resource_name;    // 资源名称（如会议室A）
    std:: string activity_name;    // 活动名称
    TimeSlot time_slot;          // 时间段
    int priority;                // 优先级（1-10，10为最高）
    std::string contact_info;    // 联系信息
    
    Reservation() : id(0), time_slot("", ""), priority(5) {}  // 默认构造函数
    Reservation(int id, const std:: string& resource, const std::string& activity,
               const TimeSlot& slot, int priority = 5, const std::string& contact = "");
};

// 冲突信息结构
struct ConflictInfo {
    std::vector<Reservation> conflicting_reservations;
    std::string resource_name;
    TimeSlot conflict_period;
    std::string suggestion;      // 解决建议
    
    ConflictInfo(const std::string& resource, const TimeSlot& period);
};

class ConflictDetector {
private: 
    std::map<std::string, std::unique_ptr<SegmentTree>> resource_trees;  // 每个资源一个线段树
    std::map<int, Reservation> reservations;                             // 预约ID映射
    std:: set<std::string> available_resources;                          // 可用资源列表
    
    int next_reservation_id;
    bool auto_resolve_enabled;    // 是否启用自动解决冲突

public:
    ConflictDetector();
    ~ConflictDetector();
    
    // 禁用拷贝
    ConflictDetector(const ConflictDetector&) = delete;
    ConflictDetector& operator=(const ConflictDetector&) = delete;
    
    // 初始化
    bool initialize(const std::vector<std::string>& resources);
    void addResource(const std:: string& resource_name);
    void removeResource(const std::string& resource_name);
    std::vector<std::string> getAvailableResources() const;
    
    // 预约管理
    int addReservation(const std::string& resource, const std::string& activity,
                      const std::string& start_time, const std::string& end_time,
                      int priority = 5, const std::string& contact = "");
    int addReservation(const Reservation& reservation);
    bool removeReservation(int reservation_id);
    bool updateReservation(int reservation_id, const Reservation& new_reservation);
    
    // 冲突检测
    bool hasConflict(const std::string& resource, const TimeSlot& time_slot) const;
    std::vector<ConflictInfo> detectAllConflicts() const;
    std::vector<Reservation> findConflictingReservations(const std::string& resource, 
                                                        const TimeSlot& time_slot) const;
    
    // 智能调度
    std::vector<std::string> findAvailableResources(const TimeSlot& time_slot) const;
    std:: vector<TimeSlot> suggestAlternativeSlots(const std::string& resource, 
                                                 const TimeSlot& preferred_slot,
                                                 int duration_minutes = 60) const;
    std::string findBestResource(const TimeSlot& time_slot, int min_priority = 1) const;
    
    // 冲突解决
    void enableAutoResolve(bool enable = true);
    std::vector<std::string> generateResolutionSuggestions(const ConflictInfo& conflict) const;
    bool resolveConflictByPriority(const std::string& resource, const TimeSlot& time_slot);
    bool resolveConflictByRescheduling(int low_priority_reservation_id);
    
    // 查询和统计
    std::vector<Reservation> getReservationsByResource(const std::string& resource) const;
    std::vector<Reservation> getReservationsByTimeRange(const TimeSlot& time_range) const;
    std::vector<Reservation> getAllReservations() const;
    
    // 工具方法
    void printResourceUsage() const;
    void printConflictReport() const;
    void printSchedule(const std::string& resource = "") const;
    
    // 统计分析
    double getResourceUtilization(const std::string& resource) const;
    std::map<std::string, int> getResourceUsageStats() const;
    int getTotalReservations() const;
    
private:
    // 内部辅助方法
    int parseTimeToMinutes(const std::string& time_str) const;
    std::string minutesToTimeString(int minutes) const;
    void updateResourceTree(const std::string& resource, const Reservation& reservation, bool add = true);
    std::vector<TimeSlot> findFreeSlots(const std::string& resource, int duration_minutes = 60) const;
    bool isValidTimeFormat(const std::string& time_str) const;
};

#endif // CONFLICT_DETECTOR_H