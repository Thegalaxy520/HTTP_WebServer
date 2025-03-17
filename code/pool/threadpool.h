//
// Created by moon on 25-3-16.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
// 线程池类，用于管理多个工作线程并行处理任务
class ThreadPool {
public:
    // 构造函数，默认创建8个线程
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()/*make_shared用于创建共享指针*/) {
            assert(threadCount > 0);  // 确保线程数合法
            for(size_t i = 0; i < threadCount; i++) {
                // 创建工作线程（立即detach，不等待线程结束）
                std::thread([pool = pool_] {  // 捕获共享的Pool对象
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    // 从线程池中获取互斥锁，其他线程获取时会被已经获取的某一线程阻塞
                    while(true) {  // 线程主循环
                        // 有任务时处理任务
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());  // 提取任务
                            //std::move将左值转换成可被引用的右值
                            pool->tasks.pop();  // 移除已取出的任务
                            locker.unlock();    // 释放锁，允许其他线程操作队列
                            task();             // 执行任务（无锁状态下执行）
                            locker.lock();      // 重新加锁准备下一次循环
                        }
                        // 线程池关闭时退出循环
                        else if(pool->isClosed) break;
                        // 无任务时等待条件变量唤醒
                        else pool->cond.wait(locker);  // 自动释放锁并进入等待
                    }
                }).detach();  // 分离线程（生命周期与线程池绑定）
            }
    }

    // 默认构造函数
    ThreadPool() = default;

    // 移动构造函数
    ThreadPool(ThreadPool&&) = default;

    // 析构函数：关闭线程池并唤醒所有线程
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {  // 检查pool_是否有效
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);  // 加锁修改isClosed
                pool_->isClosed = true;  // 设置关闭标志（注意：isClosed未初始化）
            }
            pool_->cond.notify_all();  // 唤醒所有等待线程
        }
    }

    // 添加任务到队列（支持完美转发）
    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);  // 加锁保护队列
            pool_->tasks.emplace(std::forward<F>(task));  // 将任务加入队列
        }
        pool_->cond.notify_one();  // 唤醒一个等待线程
    }

private:
    // 线程池共享的内部数据结构
    struct Pool {
        std::mutex mtx;                          // 互斥锁
        std::condition_variable cond;            // 条件变量
        bool isClosed;                           // 关闭标志（未初始化）
        std::queue<std::function<void()>> tasks; // 任务队列（存储可调用对象）
    };
    std::shared_ptr<Pool> pool_;  // 共享的Pool对象（允许多个ThreadPool实例共享状态）
};
#endif //THREADPOOL_H
