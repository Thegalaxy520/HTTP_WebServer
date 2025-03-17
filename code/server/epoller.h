//
// Created by moon on 25-3-17.
//

#ifndef EPOLLER_H
#define EPOLLER_H

#include<cstdint>
#include<vector>
#include<cassert>
#include<sys/epoll.h>
#include<unistd.h>


// Epoll 事件轮询器封装类，用于高效管理 I/O 事件
class Epoller {
public:
	// 构造函数：创建 epoll 实例并初始化事件容器
	// 参数：maxEvent - 预分配的最大可处理事件数（默认1024）
	explicit Epoller(int maxEvent = 1024);

	// 析构函数：关闭 epoll 文件描述符
	~Epoller();

	// 添加文件描述符到 epoll 监控
	// 参数：fd - 要监控的文件描述符
	//      events - 监控的事件标志（EPOLLIN/EPOLLOUT 等）
	// 返回：操作是否成功
	bool AddFd(int fd, uint32_t events);
	// 修改已监控的文件描述符事件
	// 参数：fd - 已存在的文件描述符
	//      events - 新的事件标志
	// 返回：操作是否成功
	bool ModFd(int fd, uint32_t events);
	// 从 epoll 监控移除文件描述符
	// 参数：fd - 要移除的文件描述符
	// 返回：操作是否成功
	bool DelFd(int fd);
	// 等待事件触发
	// 参数：timeoutMs - 超时时间（毫秒，-1表示阻塞等待）
	// 返回：就绪事件数量（错误返回-1）
	int Wait(int timeoutMs = -1);
	// 获取第 i 个就绪事件对应的文件描述符
	// 参数：i - 事件索引（需 0 <= i < Wait() 返回值）
	int GetEventFd(size_t i) const;
	// 获取第 i 个就绪事件的事件标志
	// 参数：i - 事件索引（需 0 <= i < Wait() 返回值）
	uint32_t GetEvents(size_t i) const;
private:
	int epollFd;                        // epoll 实例的文件描述符
	std::vector<struct epoll_event> epollEvents; // 存储就绪事件的容器
};


#endif //EPOLLER_H
