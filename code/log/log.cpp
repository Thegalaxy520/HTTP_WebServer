/*
 * 日志系统实现文件
 * 设计要点：
 * 1. 支持多线程安全写入
 * 2. 异步日志写入，提高性能
 * 3. 日志级别控制
 * 4. 日志文件按日期和大小分割
 */
#include "log.h"

using namespace std;

// 构造函数初始化日志系统
Logger::Logger() {
    currentLineCount_ = 0;       // 初始化日志行数
    isAsyncMode_ = false;        // 默认同步模式
    writeThread_ = nullptr;      // 异步写线程指针
    logQueue_ = nullptr;         // 日志队列指针
    currentDay_ = 0;             // 当前日期
    logFile_ = nullptr;          // 日志文件指针
}

// 析构函数，确保资源释放
Logger::~Logger() {
    if (writeThread_ && writeThread_->joinable()) {  // 异步模式下的线程处理
        while (!logQueue_->IsEmpty()) {               // 确保队列清空
            logQueue_->NotifyOneConsumer();
        }
        logQueue_->Shutdown();                         // 关闭队列
        writeThread_->join();                       // 等待线程结束
    }
    if (logFile_) {                                 // 关闭日志文件
        lock_guard<mutex> lock(logMutex_);
        Flush();
        fclose(logFile_);
    }
}

// 获取当前日志级别
int Logger::GetLogLevel() {
    lock_guard<mutex> lock(logMutex_);
    return logLevel_;
}

// 设置日志级别
void Logger::SetLogLevel(int level) {
    lock_guard<mutex> lock(logMutex_);
    logLevel_ = level;
}

// 初始化日志系统
void Logger::Initialize(int level, const char* path, const char* suffix, int maxQueueSize) {
    isLogOpen_ = true;                            // 开启日志系统
    logLevel_ = level;                            // 设置日志级别
    if (maxQueueSize > 0) {                       // 异步模式配置
        isAsyncMode_ = true;
        if (!logQueue_) {
            unique_ptr<BlockingDeque<std::string>> newQueue(new BlockingDeque<std::string>);
            logQueue_ = move(newQueue);           // 初始化日志队列

            unique_ptr<std::thread> newThread(new thread(AsyncWriteThread));
            writeThread_ = move(newThread);       // 启动异步写线程
        }
    } else {
        isAsyncMode_ = false;                     // 同步模式
    }

    currentLineCount_ = 0;                        // 重置日志行数

    time_t timer = time(nullptr);                 // 获取当前时间
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    logPath_ = path;                              // 设置日志路径
    logSuffix_ = suffix;                          // 设置日志文件后缀
    char fileName[LOG_NAME_MAX_LENGTH] = {0};     // 生成日志文件名
    snprintf(fileName, LOG_NAME_MAX_LENGTH - 1, "%s/%04d_%02d_%02d%s",
             logPath_, t.tm_year  + 1900, t.tm_mon  + 1, t.tm_mday,  logSuffix_);
    currentDay_ = t.tm_mday;                       // 记录当前日期

    {
        lock_guard<mutex> lock(logMutex_);        // 加锁保护
        logBuffer_.ResetReadWritePositions();                // 清空缓冲区
        if (logFile_) {
            Flush();
            fclose(logFile_);                     // 关闭旧日志文件
        }

        logFile_ = fopen(fileName, "a");          // 打开新日志文件
        if (logFile_ == nullptr) {
            mkdir(logPath_, 0777);               // 创建日志目录
            logFile_ = fopen(fileName, "a");
        }
        assert(logFile_ != nullptr);             // 确保文件打开成功
    }
}

// 写入日志
void Logger::WriteLog(int level, const char *format, ...) {
    struct timeval now = {0, 0};                 // 获取当前时间
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;                              // 可变参数列表

    // 日志文件分割逻辑
    if (currentDay_ != t.tm_mday  || (currentLineCount_ && (currentLineCount_ % MAX_LOG_LINES == 0))) {
        unique_lock<mutex> lock(logMutex_);
        lock.unlock();

        char newFile[LOG_NAME_MAX_LENGTH];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year  + 1900, t.tm_mon  + 1, t.tm_mday);

        if (currentDay_ != t.tm_mday)  {          // 按日期分割
            snprintf(newFile, LOG_NAME_MAX_LENGTH - 72, "%s/%s%s", logPath_, tail, logSuffix_);
            currentDay_ = t.tm_mday;
            currentLineCount_ = 0;
        } else {                                 // 按大小分割
            snprintf(newFile, LOG_NAME_MAX_LENGTH - 72, "%s/%s-%d%s", logPath_, tail, (currentLineCount_ / MAX_LOG_LINES), logSuffix_);
        }

        lock.lock();
        Flush();
        fclose(logFile_);
        logFile_ = fopen(newFile, "a");          // 打开新日志文件
        assert(logFile_ != nullptr);
    }

    {
        unique_lock<mutex> lock(logMutex_);      // 加锁保护
        currentLineCount_++;                     // 日志行数递增
        int n = snprintf(logBuffer_.GetWritePointer(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year  + 1900, t.tm_mon  + 1, t.tm_mday,
                         t.tm_hour,  t.tm_min,  t.tm_sec,  now.tv_usec);   // 时间戳
        logBuffer_.CommitWrite(n);
        AddLogLevelTitle(level);                 // 添加日志级别

        va_start(vaList, format);                // 格式化日志内容
        int m = vsnprintf(logBuffer_.GetWritePointer(), logBuffer_.GetWritableBytes(), format, vaList);
        va_end(vaList);

        logBuffer_.CommitWrite(m);
        logBuffer_.Append("\n\0", 2);            // 添加换行符

        if (isAsyncMode_ && logQueue_ && !logQueue_->IsFull()) {  // 异步模式
            logQueue_->Append(logBuffer_.ReadAllAsString());
        } else {                                 // 同步模式
            fputs(logBuffer_.GetReadPointer(), logFile_);
        }
        logBuffer_.ResetReadWritePositions();                // 清空缓冲区
    }
}

// 添加日志级别标题
void Logger::AddLogLevelTitle(int level) {
    switch (level) {
    case 0:
        logBuffer_.Append("[debug]: ", 9);       // 调试级别
        break;
    case 1:
        logBuffer_.Append("[info] : ", 9);       // 信息级别
        break;
    case 2:
        logBuffer_.Append("[warn] : ", 9);       // 警告级别
        break;
    case 3:
        logBuffer_.Append("[error]: ", 9);       // 错误级别
        break;
    default:
        logBuffer_.Append("[info] : ", 9);       // 默认信息级别
        break;
    }
}

// 刷新日志缓冲区
void Logger::Flush() {
    if (isAsyncMode_) {
        logQueue_->NotifyOneConsumer();
    }
    fflush(logFile_);                            // 刷新文件缓冲区
}

// 异步写日志实现
void Logger::PerformAsyncWrite() {
    string logEntry = "";
    while (logQueue_->Take(logEntry)) {           // 从队列中取出日志
        lock_guard<mutex> lock(logMutex_);
        fputs(logEntry.c_str(), logFile_);       // 写入日志文件
    }
}

// 获取单例实例
Logger* Logger::GetInstance() {
    static Logger instance;
    return &instance;
}

// 异步写日志线程函数
void Logger::AsyncWriteThread() {
    Logger::GetInstance()->PerformAsyncWrite();
}