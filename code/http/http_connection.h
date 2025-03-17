/**
 * @file http_conn.h
 * @brief HTTP 连接处理类（基于 Reactor 模式封装）
 * @details 管理单个 HTTP 连接的读写、协议解析及资源管理
 * @license Apache 2.0
 */

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>

//#include "../log/log.h"
//#include "../pool/sqlconnRAII.h"
//#include "../buffer/buffer.h"
//#include "httprequest.h"
//#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    /// @brief 初始化连接（通常在 accept 新连接后调用）
    /// @param sock_fd 已建立连接的 socket 文件描述符
    /// @param addr 对端地址信息
    void Init(int sock_fd, const sockaddr_in& addr);

    /// @brief 从 socket 读取数据到读缓冲区
    /// @param[out] saved_errno 保存错误码（若发生错误）
    /// @return 读取的字节数（-1 表示错误）
    ssize_t Read(int* saved_errno);

    /// @brief 将写缓冲区数据写入 socket
    /// @param[out] saved_errno 保存错误码（若发生错误）
    /// @return 写入的字节数（-1 表示错误）
    ssize_t Write(int* saved_errno);

    /// @brief 关闭连接并释放资源
    void Close();

    /// @brief 获取当前连接的文件描述符
    int GetFd() const;

    /// @brief 获取对端端口号（主机字节序）
    int GetPort() const;

    /// @brief 获取对端 IP 地址字符串（点分十进制）
    const char* GetIp() const;

    /// @brief 获取对端地址结构副本
    sockaddr_in GetAddr() const;

    /// @brief 处理 HTTP 请求（解析请求并生成响应）
    /// @return 是否处理成功
    bool Process();

    /// @brief 获取待发送数据总字节数（iov 聚合）
    int PendingWriteBytes() const {
        return iovec_[0].iov_len + iovec_[1].iov_len;
    }

//    /// @brief 检查是否保持长连接
//    bool IsKeepAlive() const {
//        return request_.IsKeepAlive();
//    }
//
//    static bool use_et_mode;           ///< 是否使用边缘触发模式（ET）
//    static const char* resource_dir;   ///< 静态资源根目录路径
//    static std::atomic<int> conn_count;///< 当前活跃连接数统计
//
private:
    int sock_fd_;                      ///< 连接对应的 socket 描述符
    struct sockaddr_in client_addr_;   ///< 客户端地址信息

    bool is_closed_;                   ///< 连接是否已关闭标志

    int iovec_cnt_;                    ///< 当前使用的 iovec 数量（1 或 2）
    struct iovec iovec_[2];            ///< 分散写结构数组（响应头+文件数据）

//    Buffer read_buff_;                 ///< 读缓冲区（存储原始请求数据）
//    Buffer write_buff_;                ///< 写缓冲区（存储响应头数据）
//
//    HttpRequest request_;              ///< HTTP 请求解析器
//    HttpResponse response_;            ///< HTTP 响应生成器
};

#endif // HTTP_CONN_H