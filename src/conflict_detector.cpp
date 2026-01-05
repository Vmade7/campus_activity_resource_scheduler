#include "../include/conflict_detector.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

// TimeSlot 实现

TimeSlot::TimeSlot(const std:: string& start, const std::string& end) 
    : start_time(start), end_time(end) {
    
    // 简化时间解析：假设格式为 "YYYY-MM-DD HH:MM" 或 "HH:MM"
    size_t space_pos = start. find(' ');
    std::string start_time_only = (space_pos != std::string::npos) ? start.substr(space_pos + 1) : start;
    
    space_pos = end.find(' ');
    std::string end_time_only = (space_pos != std::string:: npos) ? end.substr(space_pos + 1) : end;
    
    // 转换为分钟数（简化实现）
    std::istringstream start_ss(start_time_only);
    std::string start_hour_str, start_min_str;
    std::getline(start_ss, start_hour_str, ':');
    std::getline(start_ss, start_min_str);
    
    std::istringstream end_ss(end_time_only);
    std::string end_hour_str, end_min_str;
    std::getline(end_ss, end_hour_str, ':');
    std::getline(end_ss, end_min_str);
    
    // 添加错误处理
    if (start_hour_str.empty() || start_min_str.empty()) {
        start_minutes = 0;
    } else {
        start_minutes = std::stoi(start_hour_str) * 60 + std::stoi(start_min_str);
    }
    
    if (end_hour_str.empty() || end_min_str.empty()) {
        end_minutes = 0;
    } else {
        end_minutes = std::stoi(end_hour_str) * 60 + std::stoi(end_min_str);
    }
}

bool TimeSlot:: overlaps(const TimeSlot& other) const {
    return !(end_minutes <= other.start_minutes || start_minutes >= other.end_minutes);
}

std::string TimeSlot::toString() const {
    return start_time + " - " + end_time;
}

// Reservation 实现

Reservation:: Reservation(int id, const std::string& resource, const std:: string& activity,
                        const TimeSlot& slot, int priority, const std::string& contact)
    : id(id), resource_name(resource), activity_name(activity), time_slot(slot),
      priority(priority), contact_info(contact) {}

// ConflictInfo 实现

ConflictInfo::ConflictInfo(const std:: string& resource, const TimeSlot& period)
    : resource_name(resource), conflict_period(period) {}

// ConflictDetector 实现

ConflictDetector::ConflictDetector() : next_reservation_id(1), auto_resolve_enabled(false) {}

ConflictDetector:: ~ConflictDetector() = default;

bool ConflictDetector::initialize(const std::vector<std:: string>& resources) {
    std::cout << "初始化冲突检测器..." << std::endl;
    
    for (const auto& resource : resources) {
        addResource(resource);
    }
    
    std::cout << "冲突检测器初始化完成，管理 " << resources.size() << " 个资源" << std::endl;
    return true;
}

void ConflictDetector::addResource(const std::string& resource_name) {
    available_resources.insert(resource_name);
    resource_trees[resource_name].reset(new SegmentTree(1440)); // 一天1440分钟
    
    std::cout << "添加资源:  " << resource_name << std::endl;
}

void ConflictDetector:: removeResource(const std::string& resource_name) {
    available_resources.erase(resource_name);
    resource_trees. erase(resource_name);
    
    std::cout << "移除资源: " << resource_name << std::endl;
}

std::vector<std::string> ConflictDetector::getAvailableResources() const {
    return std::vector<std:: string>(available_resources.begin(), available_resources.end());
}

// 预约管理

int ConflictDetector::addReservation(const std::string& resource, const std::string& activity,
                                    const std::string& start_time, const std::string& end_time,
                                    int priority, const std::string& contact) {
    
    TimeSlot slot(start_time, end_time);
    Reservation reservation(next_reservation_id++, resource, activity, slot, priority, contact);
    
    return addReservation(reservation);
}

int ConflictDetector::addReservation(const Reservation& reservation) {
    // 检查资源是否存在
    if (available_resources.find(reservation.resource_name) == available_resources.end()) {
        std::cerr << "资源不存在: " << reservation.resource_name << std::endl;
        return -1;
    }
    
    // 检查冲突
    if (hasConflict(reservation.resource_name, reservation.time_slot)) {
        if (auto_resolve_enabled) {
            std::cout << "检测到冲突，尝试自动解决..." << std:: endl;
            if (! resolveConflictByPriority(reservation. resource_name, reservation.time_slot)) {
                std:: cerr << "无法自动解决冲突" << std::endl;
                return -1;
            }
        } else {
            auto conflicts = findConflictingReservations(reservation.resource_name, reservation. time_slot);
            std::cerr << "资源冲突 - " << reservation.resource_name << " 在 " 
                     << reservation.time_slot. toString() << std::endl;
            for (const auto& conflict : conflicts) {
                std::cerr << "  与活动冲突: " << conflict.activity_name 
                         << " (" << conflict.time_slot.toString() << ")" << std::endl;
            }
            return -1;
        }
    }
    
    // 添加预约
    int reservation_id = reservation.id;
    reservations[reservation_id] = reservation;
    updateResourceTree(reservation.resource_name, reservation, true);
    
    std::cout << "预约成功:  " << reservation. activity_name << " @ " << reservation.resource_name
             << " (" << reservation.time_slot.toString() << ") [ID: " << reservation_id << "]" << std::endl;
    
    return reservation_id;
}

bool ConflictDetector:: removeReservation(int reservation_id) {
    auto it = reservations.find(reservation_id);
    if (it == reservations.end()) {
        std:: cerr << "预约不存在: ID=" << reservation_id << std::endl;
        return false;
    }
    
    const Reservation& reservation = it->second;
    updateResourceTree(reservation. resource_name, reservation, false);
    reservations.erase(it);
    
    std::cout << "预约已取消: " << reservation. activity_name << " [ID: " << reservation_id << "]" << std::endl;
    return true;
}

bool ConflictDetector::updateReservation(int reservation_id, const Reservation& new_reservation) {
    if (removeReservation(reservation_id)) {
        Reservation updated_reservation = new_reservation;
        updated_reservation.id = reservation_id;
        return addReservation(updated_reservation) != -1;
    }
    return false;
}

// 冲突检测

bool ConflictDetector::hasConflict(const std::string& resource, const TimeSlot& time_slot) const {
    auto conflicts = findConflictingReservations(resource, time_slot);
    return !conflicts.empty();
}

std::vector<ConflictInfo> ConflictDetector::detectAllConflicts() const {
    std::vector<ConflictInfo> all_conflicts;
    
    for (const auto& resource : available_resources) {
        auto resource_reservations = getReservationsByResource(resource);
        
        // 检查该资源的所有预约间是否有冲突
        for (size_t i = 0; i < resource_reservations.size(); ++i) {
            for (size_t j = i + 1; j < resource_reservations.size(); ++j) {
                const auto& res1 = resource_reservations[i];
                const auto& res2 = resource_reservations[j];
                
                if (res1.time_slot.overlaps(res2.time_slot)) {
                    ConflictInfo conflict(resource, res1.time_slot);
                    conflict.conflicting_reservations = {res1, res2};
                    conflict.suggestion = "建议重新安排其中一个活动的时间或更换场地";
                    all_conflicts.push_back(conflict);
                }
            }
        }
    }
    
    return all_conflicts;
}

std::vector<Reservation> ConflictDetector::findConflictingReservations(const std:: string& resource, 
                                                                      const TimeSlot& time_slot) const {
    std:: vector<Reservation> conflicts;
    
    auto resource_reservations = getReservationsByResource(resource);
    for (const auto& reservation : resource_reservations) {
        if (reservation.time_slot.overlaps(time_slot)) {
            conflicts.push_back(reservation);
        }
    }
    
    return conflicts;
}

// 智能调度

std:: vector<std::string> ConflictDetector::findAvailableResources(const TimeSlot& time_slot) const {
    std::vector<std::string> available;
    
    for (const auto& resource : available_resources) {
        if (! hasConflict(resource, time_slot)) {
            available. push_back(resource);
        }
    }
    
    std::cout << "时间段 " << time_slot.toString() << " 可用资源: " << available.size() << " 个" << std::endl;
    return available;
}

std::vector<TimeSlot> ConflictDetector:: suggestAlternativeSlots(const std:: string& resource, 
                                                              const TimeSlot& preferred_slot,
                                                              int duration_minutes) const {
    std:: vector<TimeSlot> suggestions;
    
    // 简化实现：建议前后各1小时的时间段
    int original_start = preferred_slot. start_minutes;
    int original_duration = preferred_slot. end_minutes - preferred_slot.start_minutes;
    
    // 向前建议
    int new_start = original_start - 60; // 提前1小时
    if (new_start >= 0) {
        TimeSlot earlier_slot(minutesToTimeString(new_start), 
                             minutesToTimeString(new_start + original_duration));
        if (!hasConflict(resource, earlier_slot)) {
            suggestions.push_back(earlier_slot);
        }
    }
    
    // 向后建议
    new_start = original_start + 60; // 推迟1小时
    if (new_start + original_duration <= 1440) {
        TimeSlot later_slot(minutesToTimeString(new_start),
                           minutesToTimeString(new_start + original_duration));
        if (!hasConflict(resource, later_slot)) {
            suggestions.push_back(later_slot);
        }
    }
    
    std::cout << "为资源 " << resource << " 建议 " << suggestions.size() << " 个替代时间段" << std::endl;
    return suggestions;
}

std::string ConflictDetector::findBestResource(const TimeSlot& time_slot, int min_priority) const {
    auto available = findAvailableResources(time_slot);
    
    if (available. empty()) {
        return "";
    }
    
    // 简化实现：返回第一个可用资源
    std::cout << "🎯 推荐最佳资源: " << available[0] << std::endl;
    return available[0];
}

// 冲突解决

void ConflictDetector::enableAutoResolve(bool enable) {
    auto_resolve_enabled = enable;
    std::cout << "自动冲突解决已" << (enable ? "启用" : "禁用") << std::endl;
}

std::vector<std::string> ConflictDetector::generateResolutionSuggestions(const ConflictInfo& conflict) const {
    std::vector<std::string> suggestions;
    
    suggestions.push_back("更换到其他可用资源");
    suggestions.push_back("调整活动时间避开冲突");
    suggestions.push_back("协商共享资源使用");
    suggestions.push_back("取消优先级较低的活动");
    
    return suggestions;
}

bool ConflictDetector::resolveConflictByPriority(const std::string& resource, const TimeSlot& time_slot) {
    auto conflicts = findConflictingReservations(resource, time_slot);
    
    if (conflicts. empty()) return true;
    
    // 找到优先级最低的预约
    int lowest_priority_id = -1;
    int lowest_priority = 11; // 优先级1-10，11表示无效值
    
    for (const auto& conflict : conflicts) {
        if (conflict. priority < lowest_priority) {
            lowest_priority = conflict.priority;
            lowest_priority_id = conflict.id;
        }
    }
    
    if (lowest_priority_id != -1) {
        std::cout << "根据优先级解决冲突，取消预约 ID:  " << lowest_priority_id << std:: endl;
        return removeReservation(lowest_priority_id);
    }
    
    return false;
}

bool ConflictDetector::resolveConflictByRescheduling(int low_priority_reservation_id) {
    auto it = reservations.find(low_priority_reservation_id);
    if (it == reservations.end()) return false;
    
    const Reservation& reservation = it->second;
    
    // 寻找替代时间段
    auto alternatives = suggestAlternativeSlots(reservation.resource_name, reservation.time_slot);
    
    if (! alternatives.empty()) {
        // 使用第一个替代方案
        Reservation new_reservation = reservation;
        new_reservation.time_slot = alternatives[0];
        
        std::cout << "重新调度预约到: " << alternatives[0]. toString() << std::endl;
        return updateReservation(low_priority_reservation_id, new_reservation);
    }
    
    return false;
}

// 查询和统计

std::vector<Reservation> ConflictDetector::getReservationsByResource(const std::string& resource) const {
    std::vector<Reservation> resource_reservations;
    
    for (const auto& pair : reservations) {
        if (pair.second.resource_name == resource) {
            resource_reservations.push_back(pair.second);
        }
    }
    
    return resource_reservations;
}

std::vector<Reservation> ConflictDetector::getReservationsByTimeRange(const TimeSlot& time_range) const {
    std::vector<Reservation> time_reservations;
    
    for (const auto& pair : reservations) {
        if (pair.second. time_slot.overlaps(time_range)) {
            time_reservations.push_back(pair. second);
        }
    }
    
    return time_reservations;
}

std::vector<Reservation> ConflictDetector::getAllReservations() const {
    std:: vector<Reservation> all_reservations;
    
    for (const auto& pair : reservations) {
        all_reservations.push_back(pair.second);
    }
    
    return all_reservations;
}

// 工具方法

void ConflictDetector::printResourceUsage() const {
    std::cout << "=== 资源使用情况 ===" << std::endl;
    
    for (const auto& resource : available_resources) {
        auto resource_reservations = getReservationsByResource(resource);
        double utilization = getResourceUtilization(resource);
        
        std::cout << " " << resource << ": " << resource_reservations.size() 
                 << " 个预约, 利用率 " << std::fixed << std::setprecision(1) 
                 << (utilization * 100) << "%" << std::endl;
    }
}

void ConflictDetector::printConflictReport() const {
    std::cout << "=== 冲突检测报告 ===" << std::endl;
    
    auto conflicts = detectAllConflicts();
    
    if (conflicts.empty()) {
        std::cout << "当前无资源冲突" << std::endl;
    } else {
        std::cout << "发现 " << conflicts.size() << " 个冲突:" << std::endl;
        
        for (const auto& conflict : conflicts) {
            std::cout << "  " << conflict.resource_name 
                     << " 在 " << conflict.conflict_period.toString() << std::endl;
            for (const auto& res : conflict.conflicting_reservations) {
                std::cout << "    - " << res.activity_name 
                         << " (优先级: " << res.priority << ")" << std::endl;
            }
            std::cout << "    " << conflict.suggestion << std::endl;
        }
    }
}

void ConflictDetector::printSchedule(const std::string& resource) const {
    std::cout << "=== 资源预约日程 ===" << std::endl;
    
    if (resource.empty()) {
        // 打印所有资源的日程
        for (const auto& res : available_resources) {
            printSchedule(res);
            std::cout << std::endl;
        }
        return;
    }
    
    std::cout << " " << resource << ":" << std::endl;
    
    auto resource_reservations = getReservationsByResource(resource);
    std::sort(resource_reservations.begin(), resource_reservations.end(),
              [](const Reservation& a, const Reservation& b) {
                  return a.time_slot.start_minutes < b.time_slot.start_minutes;
              });
    
    if (resource_reservations.empty()) {
        std::cout << "  暂无预约" << std::endl;
    } else {
        for (const auto& reservation : resource_reservations) {
            std::cout << "  " << reservation.time_slot.toString() 
                     << " - " << reservation. activity_name 
                     << " (优先级:  " << reservation.priority << ")" << std::endl;
        }
    }
}

// 统计分析

double ConflictDetector::getResourceUtilization(const std:: string& resource) const {
    auto resource_reservations = getReservationsByResource(resource);
    
    int total_reserved_minutes = 0;
    for (const auto& reservation : resource_reservations) {
        total_reserved_minutes += (reservation.time_slot.end_minutes - reservation.time_slot.start_minutes);
    }
    
    // 假设工作时间为8小时 = 480分钟
    double utilization = static_cast<double>(total_reserved_minutes) / 480.0;
    return std::min(utilization, 1.0); // 最大100%
}

std::map<std::string, int> ConflictDetector::getResourceUsageStats() const {
    std::map<std::string, int> stats;
    
    for (const auto& resource : available_resources) {
        stats[resource] = getReservationsByResource(resource).size();
    }
    
    return stats;
}

int ConflictDetector::getTotalReservations() const {
    return reservations.size();
}

// 私有辅助方法

int ConflictDetector::parseTimeToMinutes(const std::string& time_str) const {
    std::istringstream ss(time_str);
    std::string hour_str, minute_str;
    std::getline(ss, hour_str, ':');
    std::getline(ss, minute_str);
    
    int hours = std::stoi(hour_str);
    int minutes = std::stoi(minute_str);
    
    return hours * 60 + minutes;
}

std::string ConflictDetector::minutesToTimeString(int minutes) const {
    int hours = minutes / 60;
    int mins = minutes % 60;
    
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << mins;
    
    return ss.str();
}

void ConflictDetector::updateResourceTree(const std::string& resource, const Reservation& reservation, bool add) {
    auto it = resource_trees.find(resource);
    if (it == resource_trees. end()) return;
    
    int start = reservation.time_slot. start_minutes;
    int end = reservation.time_slot. end_minutes;
    
    // 使用线段树进行区间更新
    if (add) {
        it->second->addInterval(start, end);
    } else {
        it->second->removeInterval(start, end);
    }
}

std::vector<TimeSlot> ConflictDetector::findFreeSlots(const std::string& resource, int duration_minutes) const {
    std::vector<TimeSlot> free_slots;
    
    // 简化实现：查找8: 00-18:00工作时间内的空闲时段
    int work_start = 8 * 60;   // 8:00
    int work_end = 18 * 60;    // 18:00
    
    auto resource_reservations = getReservationsByResource(resource);
    std::sort(resource_reservations.begin(), resource_reservations. end(),
              [](const Reservation& a, const Reservation& b) {
                  return a. time_slot.start_minutes < b. time_slot.start_minutes;
              });
    
    int current_time = work_start;
    for (const auto& reservation : resource_reservations) {
        if (reservation.time_slot.start_minutes > current_time) {
            int free_duration = reservation.time_slot.start_minutes - current_time;
            if (free_duration >= duration_minutes) {
                TimeSlot free_slot(minutesToTimeString(current_time),
                                  minutesToTimeString(current_time + duration_minutes));
                free_slots.push_back(free_slot);
            }
        }
        current_time = std::max(current_time, reservation.time_slot.end_minutes);
    }
    
    // 检查最后一个预约后是否还有空闲时间
    if (work_end - current_time >= duration_minutes) {
        TimeSlot free_slot(minutesToTimeString(current_time),
                          minutesToTimeString(current_time + duration_minutes));
        free_slots.push_back(free_slot);
    }
    
    return free_slots;
}

bool ConflictDetector::isValidTimeFormat(const std::string& time_str) const {
    // 简单的时间格式验证
    return time_str.find(':') != std::string::npos;
}