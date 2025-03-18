/*
 * 网络通信缓冲区实现文件
 * 最后更新时间：2025-03-18 12:46 星期二
 * 设计要点：基于vector的动态内存管理、原子操作保证线程安全、支持高效I/O操作
 */

#include "buffer.h"

// 构造函数：初始化存储空间和读写位置
Buffer::Buffer(int initialSize)
    : storage_(initialSize),  // 预分配指定大小的内存
      readPosition_(0),      // 读起始位置初始化
      writePosition_(0) {}   // 写起始位置初始化

// 获取可读数据长度（写位置 - 读位置）
size_t Buffer::GetReadableBytes() const {
    return writePosition_ - readPosition_;
}

// 获取可写空间长度（总容量 - 写位置）
size_t Buffer::GetWritableBytes() const {
    return storage_.size() - writePosition_;
}

// 获取可回收空间长度（读位置之前的空间）
size_t Buffer::GetReclaimableBytes() const {
    return readPosition_;
}

// 获取当前读指针（不移动读位置）
const char* Buffer::GetReadPointer() const {
    return GetBufferStart_() + readPosition_;
}

// 消费指定长度的数据（移动读位置）
void Buffer::ConsumeData(size_t length) {
    assert(length <= GetReadableBytes());
    readPosition_ += length;  // 原子操作保证线程安全
}

// 消费数据直到指定指针位置（用于协议解析）
void Buffer::ConsumeUntil(const char* end) {
    assert(GetReadPointer() <= end);
    ConsumeData(end - GetReadPointer());
}

// 重置缓冲区（清空数据并复位指针）
void Buffer::ResetReadWritePositions() {
    bzero(&storage_[0], storage_.size());  // 清空内存内容
    readPosition_ = 0;                     // 原子读操作
    writePosition_ = 0;                    // 原子写操作
}

// 获取所有可读数据并重置缓冲区
std::string Buffer::ReadAllAsString() {
    std::string data(GetReadPointer(), GetReadableBytes());
    ResetReadWritePositions();
    return data;
}

// 获取当前写位置常量指针（用于只读访问）
const char* Buffer::GetWriteConstPointer() const {
    return GetBufferStart_() + writePosition_;
}

// 获取当前写位置可修改指针（用于数据写入）
char* Buffer::GetWritePointer() {
    return GetBufferStart_() + writePosition_;
}

// 提交已写入的数据长度（移动写位置）
void Buffer::CommitWrite(size_t length) {
    writePosition_ += length;  // 原子操作保证线程安全
}

/******************** 数据追加操作 ********************/
// 追加字符串数据
void Buffer::Append(const std::string& data) {
    Append(data.data(),  data.length());
}

// 追加二进制数据（void*类型适配）
void Buffer::Append(const void* data, size_t length) {
    assert(data);
    Append(static_cast<const char*>(data), length);
}

// 核心追加实现（内存安全版）
void Buffer::Append(const char* data, size_t length) {
    assert(data);
    EnsureWriteCapacity(length);                    // 确保空间足够
    std::copy(data, data + length, GetWritePointer()); // 高效内存拷贝
    CommitWrite(length);                            // 更新写位置
}

// 追加另一个缓冲区的可读数据
void Buffer::Append(const Buffer& srcBuffer) {
    Append(srcBuffer.GetReadPointer(), srcBuffer.GetReadableBytes());
}

/******************** I/O 操作 ********************/
// 从文件描述符读取数据（支持大文件读取）
ssize_t Buffer::ReadFromFD(int fd, int* errorCode) {
    char tempBuffer[65535];  // 临时缓冲区（64KB）
    struct iovec ioBlocks[2]; // 分散读结构体

    const size_t availableSpace = GetWritableBytes();
    // 第一块：当前缓冲区可用空间
    ioBlocks[0].iov_base = GetWritePointer();
    ioBlocks[0].iov_len = availableSpace;
    // 第二块：临时缓冲区作为溢出存储
    ioBlocks[1].iov_base = tempBuffer;
    ioBlocks[1].iov_len = sizeof(tempBuffer);

    const ssize_t bytesRead = readv(fd, ioBlocks, 2);
    if(bytesRead < 0) {
        *errorCode = errno;  // 保存错误码
        return -1;
    }
    // 处理读取结果
    if(static_cast<size_t>(bytesRead) <= availableSpace) {
        CommitWrite(bytesRead);  // 全部存入主缓冲区
    } else {
        CommitWrite(availableSpace);  // 填满主缓冲区
        Append(tempBuffer, bytesRead - availableSpace); // 剩余存入临时区
    }
    return bytesRead;  // 返回实际读取字节数
}

// 向文件描述符写入数据（支持大文件写入）
ssize_t Buffer::WriteToFD(int fd, int* errorCode) {
    size_t dataToSend = GetReadableBytes();
    ssize_t bytesWritten = write(fd, GetReadPointer(), dataToSend);
    if(bytesWritten < 0) {
        *errorCode = errno;  // 保存错误码
        return -1;
    }
    ConsumeData(bytesWritten);  // 移动读位置
    return bytesWritten;
}

/******************** 内部实现 ********************/
// 获取存储空间起始地址（可修改版本）
char* Buffer::GetBufferStart_() {
    return &*storage_.begin();  // vector迭代器转原生指针
}

// 获取存储空间起始地址（常量版本）
const char* Buffer::GetBufferStart_() const {
    return &*storage_.begin();
}

// 内存空间管理核心算法（整理或扩容）
void Buffer::ManageBufferSpace_(size_t required) {
    if(GetWritableBytes() + GetReclaimableBytes() < required) {
        // 情况1：需要扩容（每次扩容至少满足需求+1字节）
        storage_.resize(writePosition_ + required + 1);
    } else {
        // 情况2：整理现有空间（移动有效数据到头部）
        size_t storedDataLength = GetReadableBytes();
        std::copy(GetBufferStart_() + readPosition_,
                 GetBufferStart_() + writePosition_,
                 GetBufferStart_());
        // 重置读写位置
        readPosition_ = 0;
        writePosition_ = storedDataLength;
        assert(storedDataLength == GetReadableBytes());
    }
}