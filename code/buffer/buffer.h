/*
 * 网络通信缓冲区类（线程安全设计）
 * 功能：提供高效的内存管理机制，支持动态扩容和读写位置追踪
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>      // 字符串操作相关
#include <iostream>     // 输入输出流
#include <unistd.h>     // 系统调用（如write）
#include <sys/uio.h>    // 分散/聚集I/O操作
#include <vector>       // 动态数组容器
#include <atomic>       // 原子操作支持
#include <cassert>     // 断言检查

class Buffer {
public:
	// 构造函数：初始化缓冲区容量（默认1024字节）
	explicit Buffer(int initialSize = 1024);

	// 默认析构函数（使用vector自动管理内存）
	~Buffer() = default;

	// 可写空间字节数（当前写位置到缓冲区末尾）
	size_t GetWritableBytes() const;

	// 可读空间字节数（写位置与读位置之差）
	size_t GetReadableBytes() const;

	// 可回收空间字节数（读位置之前的空间）
	size_t GetReclaimableBytes() const;

	// 获取当前读位置指针（不修改读位置）
	const char *GetReadPointer() const;

	// 确保至少有len字节可写空间（自动扩容）
	void EnsureWriteCapacity(size_t len);

	// 提交已写入数据（移动写位置）
	void CommitWrite(size_t len);

	// 消费已读数据（移动读位置）
	void ConsumeData(size_t len);

	// 消费数据直到指定指针位置（用于协议解析）
	void ConsumeUntil(const char *end);

	// 重置读写位置（等效于消费所有数据）
	void ResetReadWritePositions();

	// 获取所有可读数据并转为字符串（同时重置读写位置）
	std::string ReadAllAsString();

	// 获取当前写位置常量指针
	const char *GetWriteConstPointer() const;

	// 获取当前写位置可修改指针
	char *GetWritePointer();

	// 追加数据的不同形式重载
	void Append(const std::string &data);

	void Append(const char *data, size_t len);

	void Append(const void *data, size_t len);

	void Append(const Buffer &srcBuffer);

	// 从文件描述符读取数据（支持分散读）
	ssize_t ReadFromFD(int fd, int *errorCode);

	// 向文件描述符写入数据（支持聚集写）
	ssize_t WriteToFD(int fd, int *errorCode);

private:
	// 获取底层数组起始地址（可修改版本）
	char *GetBufferStart_();

	// 获取底层数组起始地址（常量版本）
	const char *GetBufferStart_() const;

	// 内存管理核心方法：整理或扩展缓冲区
	void ManageBufferSpace_(size_t required);

	// 数据存储容器（使用vector实现动态数组）
	std::vector<char> storage;

	// 原子操作的读写位置（保证多线程安全）
	std::atomic<size_t> readPosition; // 当前读位置索引
	std::atomic<size_t> writePosition; // 当前写位置索引
};

#endif //BUFFER_H
