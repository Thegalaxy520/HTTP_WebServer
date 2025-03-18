//
// Created by moon on 25-3-18.
//

#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <cstring>
#include <cstdarg>           // 可变参数支持
#include <cassert>
#include <sys/stat.h>         // 目录操作
#include "Block_deque.h"       // 阻塞队列
#include "../buffer/buffer.h" // 缓冲区


class Logger {
public:
    // 初始化日志系统
    void Initialize(int level, const char* path = "./log",
                   const char* suffix = ".log",
                   int maxQueueCapacity = 1024);

    // 单例模式获取实例
    static Logger* GetInstance();
    // 异步写日志线程函数
    static void AsyncWriteThread();

    // 写入日志
    void WriteLog(int level, const char *format, ...);
    // 刷新日志缓冲区
    void Flush();

    // 获取当前日志级别
    int GetLogLevel();
    // 设置日志级别
    void SetLogLevel(int level);
    // 判断日志系统是否开启
    bool IsLogOpen() { return isLogOpen_; }

private:
    Logger();
    // 添加日志级别标题
    void AddLogLevelTitle(int level);
    virtual ~Logger();
    // 异步写日志实现
    void PerformAsyncWrite();

private:
    // 常量定义
    static const int LOG_PATH_MAX_LENGTH = 256;  // 日志路径最大长度
    static const int LOG_NAME_MAX_LENGTH = 256;  // 日志文件名最大长度
    static const int MAX_LOG_LINES = 50000;      // 单个日志文件最大行数

    // 日志文件相关
    const char* logPath_;      // 日志存储路径
    const char* logSuffix_;    // 日志文件后缀

    int maxLinesPerFile_;      // 单个日志文件最大行数
    int currentLineCount_;     // 当前日志文件行数
    int currentDay_;           // 当前日期（用于日志分割）

    bool isLogOpen_;           // 日志系统是否开启
    Buffer logBuffer_;         // 日志缓冲区// 日志宏定义
#define LOG_BASE(level, format, ...) \
do {\
Logger* logger = Logger::GetInstance();\
if (logger->IsLogOpen() && logger->GetLogLevel() <= level) {\
logger->WriteLog(level, format, ##__VA_ARGS__); \
logger->Flush();\
}\
} while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);
    int logLevel_;             // 当前日志级别
    bool isAsyncMode_;         // 是否异步模式

    FILE* logFile_;            // 日志文件指针
    std::unique_ptr<BlockingDeque<std::string>> logQueue_; // 日志队列
    std::unique_ptr<std::thread> writeThread_;          // 写线程
    std::mutex logMutex_;                               // 互斥锁
};

// 日志宏定义
#define LOG_BASE(level, format, ...) \
    do {\
        Logger* logger = Logger::GetInstance();\
        if (logger->IsLogOpen() && logger->GetLogLevel() <= level) {\
            logger->WriteLog(level, format, ##__VA_ARGS__); \
            logger->Flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif // LOG_SYSTEM_H
