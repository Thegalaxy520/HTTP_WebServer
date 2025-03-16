//
// Created by moon on 25-3-14.
//

#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <cassert>
#include <chrono>
//#include "../log/log.h"


typedef std::function<void()> TimeoutCallBack; // 超时回调函数类型（无参无返回值）
typedef std::chrono::high_resolution_clock Clock; // 高精度时钟
typedef std::chrono::milliseconds MS; // 时间单位（毫秒）
typedef Clock::time_point TimeStamp; // 时间点类型（用于标记超时时间）

struct TimerNode {
	int id; //唯一标识符
	TimeStamp expires; //超时时间
	TimeoutCallBack cb; //超时触发时间的回调函数
	bool operator<(const TimerNode &t) const
	{
		return expires < t.expires;
	}
};


//堆定时器
class HeapTimer {
public:
	HeapTimer() { heap.reserve(64); } //预分配内存，优化性能
	~HeapTimer() { clear(); }

	//核心接口
	void Reschedule(int id, int newExpires); // 调整指定ID的定时器超时时间
	void Schedule(int id, int timeOut, const TimeoutCallBack &cb); // 添加新定时器
	void TriggerAndRemove(int id); // 立即执行指定ID的回调并删除定时器
	void clear(); // 清空所有定时器
	void ProcessExpiredTimers(); // 处理所有已超时的定时器
	void ExtractTop(); // 删除堆顶元素（最小超时任务）
	int NextExpirationInMs(); // 计算距离下一个超时的时间（毫秒）

private:
	//删除指定位置的定时器
	void RemoveByIndex(size_t index);

	//向上调整堆（插入时）
	void HeapifyUp(size_t index);

	//向下调整堆（删除或重构时）
	bool HeapifyDown(size_t index, size_t heapSize);

	//交换堆中的两个节点并更新哈希表
	void SwapAndUpdateIndices(size_t i, size_t j);

	//堆容器
	std::vector<TimerNode> heap;
	//ID到堆索引的映射
	std::unordered_map<int, size_t> ref;
};


#endif //HEAPTIMER_H
