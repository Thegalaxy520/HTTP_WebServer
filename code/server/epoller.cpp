//
// Created by moon on 25-3-17.
//

#include "./epoller.h"

// 构造函数：创建epoll实例并初始化事件数组
Epoller::Epoller(int maxEvent)
	: epollFd(epoll_create(512)),  // 创建epoll实例，512是内核事件表初始大小（建议值）
	  epollEvents(maxEvent) {           // 预分配事件数组空间
	assert(epollFd >= 0 && !epollEvents.empty()); // 验证epoll创建成功且数组非空
}

// 析构函数：关闭epoll文件描述符
Epoller::~Epoller() {
	close(epollFd);  // 正确释放内核资源
}

bool Epoller::AddFd(int fd, uint32_t events)
{
	if(fd < 0) return false;  // 基础有效性检查
	epoll_event ev = {0};     // 初始化事件结构（C风格初始化）
	ev.data.fd = fd;          // 关联文件描述符
	ev.events = events;       // 设置监听事件
	return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev); // 系统调用
}

// 修改已监控的文件描述符事件
bool Epoller::ModFd(int fd, uint32_t events) {
	if(fd < 0) return false;
	epoll_event ev = {0};
	ev.data.fd = fd;
	ev.events = events;
	return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}
// 从epoll监控移除文件描述符
bool Epoller::DelFd(int fd) {
	if(fd < 0) return false;
	epoll_event ev = {0};  // 注意：实际DEL操作不需要event参数（可传NULL）
	return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::Wait(int timeoutMs) {
	return epoll_wait(
		epollFd,
		&epollEvents[0],  // 获取底层数组首地址
		static_cast<int>(epollEvents.size()),  // 安全转换size类型
		timeoutMs
	);
}
// 获取第i个事件的fd
int Epoller::GetEventFd(size_t i) const {
	assert(i < epollEvents.size() );  //移除epoll中堆size_t 大于0的判断，无符号数肯定大于零
	return epollEvents[i].data.fd;
}

// 获取第i个事件标志
uint32_t Epoller::GetEvents(size_t i) const {
	assert(i < epollEvents.size());  // 同上
	return epollEvents[i].events;
}