#include "segment_tree.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>

// SegmentTree 实现
SegmentTree::SegmentTree(int size) : size(size) {
    tree.resize(4 * size, 0);
    lazy.resize(4 * size, 0);
    build(0, 0, size - 1);
}

SegmentTree::~SegmentTree() {
    // 使用vector自动管理内存
}

void SegmentTree::build(int node, int start, int end) {
    if (start == end) {
        tree[node] = 0;  // 初始时间段都未占用
    } else {
        int mid = (start + end) / 2;
        build(2 * node + 1, start, mid);
        build(2 * node + 2, mid + 1, end);
        tree[node] = 0;  // 非叶子节点初始为0
    }
}

void SegmentTree::pushDown(int node, int start, int end) {
    if (lazy[node] != 0) {
        tree[node] += lazy[node];
        
        if (start != end) {  // 不是叶子节点
            lazy[2 * node + 1] += lazy[node];
            lazy[2 * node + 2] += lazy[node];
        }
        
        lazy[node] = 0;
    }
}

void SegmentTree::updateRange(int node, int start, int end, int l, int r, int val) {
    pushDown(node, start, end);
    
    if (start > r || end < l) return;  // 完全不重叠
    
    if (start >= l && end <= r) {  // 完全包含
        lazy[node] += val;
        pushDown(node, start, end);
        return;
    }
    
    // 部分重叠，递归处理
    int mid = (start + end) / 2;
    updateRange(2 * node + 1, start, mid, l, r, val);
    updateRange(2 * node + 2, mid + 1, end, l, r, val);
    
    // 更新当前节点
    pushDown(2 * node + 1, start, mid);
    pushDown(2 * node + 2, mid + 1, end);
    tree[node] = std::max(tree[2 * node + 1], tree[2 * node + 2]);
}

int SegmentTree:: queryRange(int node, int start, int end, int l, int r) {
    if (start > r || end < l) return 0;  // 完全不重叠
    
    pushDown(node, start, end);
    
    if (start >= l && end <= r) {  // 完全包含
        return tree[node];
    }
    
    // 部分重叠，递归查询
    int mid = (start + end) / 2;
    int leftMax = queryRange(2 * node + 1, start, mid, l, r);
    int rightMax = queryRange(2 * node + 2, mid + 1, end, l, r);
    
    return std::max(leftMax, rightMax);
}

void SegmentTree::addInterval(int start, int end) {
    if (start < 0 || end >= size || start > end) return;
    updateRange(0, 0, size - 1, start, end, 1);
}

void SegmentTree::removeInterval(int start, int end) {
    if (start < 0 || end >= size || start > end) return;
    updateRange(0, 0, size - 1, start, end, -1);
}

bool SegmentTree::isConflict(int start, int end) {
    if (start < 0 || end >= size || start > end) return false;
    return queryRange(0, 0, size - 1, start, end) > 0;
}

int SegmentTree:: queryOccupancy(int start, int end) {
    if (start < 0 || end >= size || start > end) return 0;
    return queryRange(0, 0, size - 1, start, end);
}

void SegmentTree::clear() {
    std::fill(tree. begin(), tree.end(), 0);
    std::fill(lazy. begin(), lazy.end(), 0);
}

void SegmentTree::printTree() const {
    std::cout << "=== 线段树结构 ===\n";
    printNode(0, 0, size - 1, 0);
}

void SegmentTree::printNode(int node, int start, int end, int depth) const {
    std::string indent(depth * 2, ' ');
    std::cout << indent << "Node " << node << " [" << start << "," << end 
              << "] value=" << tree[node] << " lazy=" << lazy[node] << "\n";
    
    if (start != end) {
        int mid = (start + end) / 2;
        printNode(2 * node + 1, start, mid, depth + 1);
        printNode(2 * node + 2, mid + 1, end, depth + 1);
    }
}

// VenueConflictDetector 实现
VenueConflictDetector::VenueConflictDetector(int timeRange) : timeRange(timeRange) {}

bool VenueConflictDetector::addActivity(const std:: string& venue, int startTime, int endTime, int activityId) {
    // 检查时间合法性
    if (startTime < 0 || endTime >= timeRange || startTime >= endTime) {
        return false;
    }
    
    // 为场地创建线段树（如果不存在）
    if (venueToTree.find(venue) == venueToTree.end()) {
        venueToTree[venue] = new SegmentTree(timeRange);
    }
    
    // 检查冲突
    if (venueToTree[venue]->isConflict(startTime, endTime)) {
        return false;  // 有冲突，无法添加
    }
    
    // 添加活动
    venueToTree[venue]->addInterval(startTime, endTime);
    return true;
}

bool VenueConflictDetector::removeActivity(const std:: string& venue, int startTime, int endTime) {
    auto it = venueToTree.find(venue);
    if (it == venueToTree.end()) {
        return false;  // 场地不存在
    }
    
    it->second->removeInterval(startTime, endTime);
    return true;
}

bool VenueConflictDetector::checkConflict(const std::string& venue, int startTime, int endTime) {
    auto it = venueToTree. find(venue);
    if (it == venueToTree. end()) {
        return false;  // 场地不存在，无冲突
    }
    
    return it->second->isConflict(startTime, endTime);
}

std::vector<std:: pair<int, int>> VenueConflictDetector:: findAvailableSlots(const std:: string& venue, int duration) {
    std::vector<std:: pair<int, int>> availableSlots;
    
    auto it = venueToTree.find(venue);
    if (it == venueToTree.end()) {
        // 场地不存在，整个时间段都可用
        availableSlots.push_back({0, timeRange - 1});
        return availableSlots;
    }
    
    // 扫描时间轴找可用时间段
    int start = 0;
    while (start + duration <= timeRange) {
        if (! it->second->isConflict(start, start + duration - 1)) {
            int end = start + duration - 1;
            // 尝试扩展时间段
            while (end + 1 < timeRange && ! it->second->isConflict(start, end + 1)) {
                end++;
            }
            availableSlots.push_back({start, end});
            start = end + 1;
        } else {
            start++;
        }
    }
    
    return availableSlots;
}