//
// Created by moon on 25-3-14.
//

#include "heaptimer.h"

HeapTimer::~HeapTimer()
{
}

void HeapTimer::Reschedule(int id, int newExpires)
{
}

void HeapTimer::Schedule(int id, int timeOut, const TimeoutCallBack &cb)
{
}

void HeapTimer::TriggerAndRemove(int id)
{
}

void HeapTimer::clear()
{
}

void HeapTimer::ProcessExpiredTimers()
{
}

void HeapTimer::ExtractTop()
{
}

int HeapTimer::NextExpirationInMs()
{
}

void HeapTimer::RemoveByIndex(size_t index)
{
}

void HeapTimer::HeapifyUp(size_t i)
{
	assert(i >= 0 && i < heap.size());
	size_t j = (i - 1) / 2;
	while (j >= 0)
	{
		if (heap[j] < heap[i]) { break; }
		SwapAndUpdateIndices(i,j);
		j = (i -1)/2;
	}
}

bool HeapTimer::HeapifyDown(size_t index, size_t n)
{
	assert(index >= 0 && index < heap.size());
	assert(n >= 0 && n <= heap.size());
	size_t i = index;
	size_t j = i * 2 + 1;
	while(j < n) {
		if(j + 1 < n && heap[j + 1] < heap[j]) j++;
		if(heap[i] < heap[j]) break;
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
