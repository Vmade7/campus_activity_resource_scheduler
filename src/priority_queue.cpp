#include "priority_queue.h"

PriorityQueue::PriorityQueue() {}

void PriorityQueue::push(int value) {
    heap.push_back(value);
    heapifyUp(heap.size() - 1);
}

int PriorityQueue::pop() {
    if (heap.empty()) return -1;  // 返回-1表示空队列
    int result = heap[0]; 
    std::swap(heap[0], heap[heap.size() - 1]);
    heap.pop_back();
    heapifyDown(0);
    return result;
}

bool PriorityQueue::isEmpty() {
    return heap.empty();
}

void PriorityQueue::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap[index] >= heap[parent]) break;
        std::swap(heap[index], heap[parent]);
        index = parent;
    }
}

void PriorityQueue::heapifyDown(int index) {
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;
    int smallest = index;

    if (leftChild < heap.size() && heap[leftChild] < heap[smallest]) {
        smallest = leftChild;
    }

    if (rightChild < heap.size() && heap[rightChild] < heap[smallest]) {
        smallest = rightChild;
    }

    if (smallest != index) {
        std::swap(heap[index], heap[smallest]);
        heapifyDown(smallest);
    }
}