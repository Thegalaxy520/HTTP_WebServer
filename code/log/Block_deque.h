//
// Created by moon on 25-3-18.
//

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <cassert>
#include <sys/time.h>

template<class TaskType>
class BlockingDeque {
public:
	explicit BlockingDeque(size_t max_capacity = 1000);

	~BlockingDeque();

	// 队列操作接口
	void Clear();

	bool IsEmpty();

	bool IsFull();

	void Shutdown();

	size_t CurrentSize();

	size_t MaxCapacity();

	TaskType GetFront();

	TaskType GetBack();

	// 生产操作
	void Append(const TaskType &task);

	void Prepend(const TaskType &task);

	// 消费操作
	bool Take(TaskType &task);

	bool TimedTake(TaskType &task, int timeout_seconds);

	// 状态刷新
	void NotifyOneConsumer();

private:
	std::deque<TaskType> task_queue; // 任务存储容器
	size_t max_capacity; // 队列最大容量
	std::mutex queue_mutex; // 队列操作互斥锁
	bool is_shutdown; // 关闭状态标志
	std::condition_variable consumer_cv; // 消费者条件变量
	std::condition_variable producer_cv; // 生产者条件变量
};

/*================ 实现部分 ================*/
template<class TaskType>
BlockingDeque<TaskType>::BlockingDeque(size_t max_capacity_)
    : max_capacity(max_capacity_) {
    assert(max_capacity > 0);
    is_shutdown = false;  // 初始化未关闭状态
}

template<class TaskType>
BlockingDeque<TaskType>::~BlockingDeque() {
    Shutdown();  // 析构时自动关闭队列
}

// 安全关闭队列（线程安全）
template<class TaskType>
void BlockingDeque<TaskType>::Shutdown() {
    {   // 限制锁作用域
        std::lock_guard<std::mutex> lock(queue_mutex);
        task_queue.clear();    // 清空待处理任务
        is_shutdown = true;    // 设置关闭标志
    }
    // 通知所有等待线程（避免死锁）
    producer_cv.notify_all();
    consumer_cv.notify_all();
}

// 唤醒一个消费者线程（用于紧急处理）
template<class TaskType>
void BlockingDeque<TaskType>::NotifyOneConsumer() {
    consumer_cv.notify_one();
}

// 清空队列内容（线程安全）
template<class TaskType>
void BlockingDeque<TaskType>::Clear() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    task_queue.clear();
}

/*---------- 队列状态查询（均带锁保护） ----------*/
template<class TaskType>
TaskType BlockingDeque<TaskType>::GetFront() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.front();
}

template<class TaskType>
TaskType BlockingDeque<TaskType>::GetBack() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.back();
}

template<class TaskType>
size_t BlockingDeque<TaskType>::CurrentSize() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.size();
}

template<class TaskType>
size_t BlockingDeque<TaskType>::MaxCapacity() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return max_capacity;
}

template<class TaskType>
bool BlockingDeque<TaskType>::IsEmpty() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.empty();
}

template<class TaskType>
bool BlockingDeque<TaskType>::IsFull() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return task_queue.size() >= max_capacity;
}

/*---------- 生产者操作 ----------*/
template<class TaskType>
void BlockingDeque<TaskType>::Append(const TaskType &task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    // 队列满时阻塞并自动释放锁
    while (task_queue.size() >= max_capacity) {
        //队列满了就进入等待，同时解锁queue_mutex;
        producer_cv.wait(lock);
    }
    task_queue.push_back(task);
    consumer_cv.notify_one();  // 通知一个消费者
}

template<class TaskType>
void BlockingDeque<TaskType>::Prepend(const TaskType &task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    while (task_queue.size() >= max_capacity) {
         task_queue.push_front(task);  // 头部插入高优先级任务
    consumer_cv.notify_one(); producer_cv.wait(lock);
    }
    task_queue.push_front(task);  // 头部插入高优先级任务
    consumer_cv.notify_one();
}

/*---------- 消费者操作 ----------*/
template<class TaskType>
bool BlockingDeque<TaskType>::Take(TaskType &task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    // 队列空时等待，避免忙等
    while (task_queue.empty()) {
        consumer_cv.wait(lock);
        if (is_shutdown) return false;  // 关闭时立即返回
    }
    task = task_queue.front();
    task_queue.pop_front();
    producer_cv.notify_one();  // 通知生产者有空位
    return true;
}

template<class TaskType>
bool BlockingDeque<TaskType>::TimedTake(TaskType &task, int timeout_seconds) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    // 带超时的条件等待
    while (task_queue.empty()) {
        if (consumer_cv.wait_for(lock, std::chrono::seconds(timeout_seconds))
            == std::cv_status::timeout) {
            return false;  // 超时未获取数据
        }
        if (is_shutdown) return false;
    }
    task = task_queue.front();
    task_queue.pop_front();
    producer_cv.notify_one();
    return true;
}
#endif //BLOCK_QUEUE_H
