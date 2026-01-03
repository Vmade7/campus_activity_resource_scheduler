#ifndef SEGMENT_TREE_H
#define SEGMENT_TREE_H

#include <vector>
#include <string>
#include <unordered_map>

class SegmentTree {
public:
    SegmentTree(int size);
    ~SegmentTree();
    
    // 添加时间段（标记为占用）
    void addInterval(int start, int end);
    
    // 移除时间段（取消占用）
    void removeInterval(int start, int end);
    
    // 检查时间段是否冲突
    bool isConflict(int start, int end);
    
    // 查询区间内的占用次数
    int queryOccupancy(int start, int end);
    
    // 工具函数
    void clear();
    void printTree() const;  // 调试用

private:
    std::vector<int> tree;      // 线段树数组
    std::vector<int> lazy;      // 懒标记数组
    int size;                   // 时间范围大小
    
    // 核心操作函数
    void build(int node, int start, int end);
    void updateRange(int node, int start, int end, int l, int r, int val);
    int queryRange(int node, int start, int end, int l, int r);
    void pushDown(int node, int start, int end);
    
    // 辅助函数
    void printNode(int node, int start, int end, int depth) const;
};

// 场地冲突检测器
class VenueConflictDetector {
public: 
    VenueConflictDetector(int timeRange = 24 * 60);  // 默认24小时，按分钟计算
    
    // 添加活动
    bool addActivity(const std::string& venue, int startTime, int endTime, int activityId);
    
    // 移除活动
    bool removeActivity(const std::string& venue, int startTime, int endTime);
    
    // 检查冲突
    bool checkConflict(const std::string& venue, int startTime, int endTime);
    
    // 查找可用时间段
    std::vector<std:: pair<int, int>> findAvailableSlots(const std::string& venue, int duration);

private:
    std::unordered_map<std::string, SegmentTree*> venueToTree;
    int timeRange;
};

#endif // SEGMENT_TREE_H