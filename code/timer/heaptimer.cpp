//
// Created by moon on 25-3-14.
//

#include "heaptimer.h"

void HeapTimer::Reschedule(int id, int newExpires)
{
	/* 调整指定id的结点 */
	assert(!heap.empty() && ref.count(id) > 0);
	heap[ref[id]].expires = Clock::now() + MS(newExpires);;
	HeapifyDown(ref[id], heap.size());
}

void HeapTimer::Schedule(int id, int timeOut, const TimeoutCallBack &cb)
{
	assert(id >= 0);
	size_t i;
	if (ref.count(id) == 0)
	{
		/* 新节点：堆尾插入，调整堆 */
		i = heap.size();
		ref[id] = i;
		heap.push_back({id, Clock::now() + MS(timeOut), cb});
		HeapifyUp(i);
	}
	else
	{
		/* 已有结点：调整堆 */
		i = ref[id];
		heap[i].expires = Clock::now() + MS(timeOut);
		heap[i].cb = cb;
		if (!HeapifyDown(i, heap.size()))
		{
			HeapifyUp(i);
		}
	}
}

void HeapTimer::TriggerAndRemove(int id)
{
	/* 删除指定id结点，并触发回调函数 */
	if (heap.empty() || ref.count(id) == 0)
	{
		return;
	}
	size_t i = ref[id];
	TimerNode node = heap[i];
	node.cb();
	RemoveByIndex(i);
}

void HeapTimer::clear()
{
	ref.clear();
	heap.clear();
}

void HeapTimer::ProcessExpiredTimers()
{
	/* 清除超时结点 */
	if (heap.empty())
	{
		return;
	}
	while (!heap.empty())
	{
		TimerNode node = heap.front();
		if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
		{
			break;
		}
		node.cb();
		ExtractTop();
	}
}

void HeapTimer::ExtractTop()
{
	assert(!heap.empty());
	RemoveByIndex(0);
}

int HeapTimer::NextExpirationInMs()
{
	ProcessExpiredTimers();
	size_t res = -1;
	if (!heap.empty())
	{
		res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
		if (res < 0) { res = 0; }
	}
	return res;
}

void HeapTimer::RemoveByIndex(size_t index)
{
	/* 删除指定位置的结点 */
	assert(!heap.empty() && index >= 0 && index < heap.size());
	/* 将要删除的结点换到队尾，然后调整堆 */
	size_t i = index;
	size_t n = heap.size() - 1;
	assert(i <= n);
	if (i < n)
	{
		SwapAndUpdateIndices(i, n);
		if (!HeapifyDown(i, n))
		{
			HeapifyUp(i);
		}
	}
	/* 队尾元素删除 */
	ref.erase(heap.back().id);
	heap.pop_back();
}

void HeapTimer::HeapifyUp(size_t i)
{
	assert(i >= 0 && i < heap.size());
	size_t j = (i - 1) / 2;
	while (j >= 0)
	{
		if (heap[j] < heap[i]) { break; }
		SwapAndUpdateIndices(i, j);
		j = (i - 1) / 2;
	}
}

bool HeapTimer::HeapifyDown(size_t index, size_t n)
{
	assert(index >= 0 && index < heap.size());
	assert(n >= 0 && n <= heap.size());
	size_t i = index;
	size_t j = i * 2 + 1;
	while (j < n)
	{
		if (j + 1 < n && heap[j + 1] < heap[j]) j++;
		if (heap[i] < heap[j]) break;
		SwapAndUpdateIndices(i, j);
		i = j;
		j = i * 2 + 1;
	}
	return i > index;
}

void HeapTimer::SwapAndUpdateIndices(size_t i, size_t j)
{
	assert(i >= 0 && i < heap.size());
	assert(j >= 0 && j < heap.size());
	std::swap(heap[i], heap[j]);
	ref[heap[i].id] = i;
	ref[heap[j].id] = j;
}
