# 🚀 Http_WebServer - 高性能C++ Web服务器  
**📅 最后更新：2025年3月18日 16:49 | [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)**   

---

## 📜 项目概览  
基于C++14实现的轻量级高并发Web服务器，通过 **Reactor多线程模型** 和 **异步I/O优化**，经WebBench压测实现 **单机13.9万QPS**（10K并发零失败）。核心模块包含日志系统、连接池、定时器等企业级组件。

---

## 🎯 核心功能  
### 五大技术亮点  
1. **多线程架构**  
   - Reactor模式 + Epoll边缘触发  
   - 线程池动态调度（支持CPU核心数自动适配）  
2. **协议解析**  
   - 正则表达式+状态机解析HTTP/1.1  
   - 支持GET/POST/HEAD方法及Keep-Alive  
3. **资源管理**  
   - RAII式数据库连接池（MySQL）  
   - 小根堆定时器自动清理超时连接  
4. **性能优化**  
   - 双缓冲异步日志（500MB/s吞吐）  
   - 零拷贝缓冲区减少内存复制  
5. **扩展能力**  
   - 模块化设计（日志/配置/协议可插拔）  
   - 预留CGI接口支持动态脚本  

---

## 🛠️ 环境要求  
| 组件                | 最低版本      | 推荐配置               |  
|---------------------|-------------|-----------------------|  
| 操作系统            | Ubuntu 18.04 | Ubuntu 22.04 LTS      |  
| 编译器              | g++ 7.0     | g++ 12.2              |  
| MySQL               | 5.7         | 8.0                   |  
| 内存                | 2GB         | 8GB DDR4 3200MHz      |  

---
## 📂 项目结构  
```bash 
├─.idea             # IDE配置文件（适用于JetBrains系列工具）
├─build             # 编译产物目录（CMake/Makefile生成的可执行文件）
├─code              # 核心源码库 
│  ├─buffer         # 网络I/O缓冲系统（支持动态扩容与零拷贝技术）
│  ├─config         # 配置管理中心（YAML解析与热加载实现）
│  ├─http           # HTTP协议栈（含请求解析/路由/响应生成模块）
│  ├─log            # 异步日志系统（分级日志+阻塞队列+滚动归档）
│  ├─pool           # 资源池（数据库连接池 & 线程池统一管理）
│  ├─server         # 服务端主逻辑（Reactor事件驱动引擎）
│  └─timer          # 定时器模块（小根堆算法实现超时管理）
├─readme.assest      # 文档资源库（含架构图/流程图等可视化素材）
├─resources         # 静态资源库（Web服务托管文件）
│  ├─css            # 层叠样式表（Bootstrap定制化方案）
│  ├─fonts          # 字体资源（WOFF2/WOFF/TTF格式）
│  ├─images         # 图像资源（WebP/PNG/SVG优化格式）
│  ├─js             # JavaScript脚本（ES6+模块化实现）
│  └─video          # 流媒体资源（H.264/WebM编码）
├─test              # 测试套件（GTest单元测试+压力测试用例）
└─webbench-1.5      # 压测工具链（定制化10万级并发测试脚本）
```
# 🚀 Http_WebServer 快速启动指南  

---

## 1. 数据库配置  
```sql 
-- 创建数据库（字符集建议使用utf8mb4）
CREATE DATABASE yourdb;
USE yourdb;
 
-- 创建用户表（与代码实现严格对应）
CREATE TABLE user (
    username CHAR(50) PRIMARY KEY,
    password CHAR(50) NOT NULL 
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
 
-- 插入测试数据（需与登录接口参数匹配）
INSERT INTO user(username, password) VALUES('test', '123456');
```

## 2. 编译与启动

~~~bash
make
./bin/server
~~~

## 3.单元测试

```bash
cd test
make
./test
```

## 4. 压力测试

![image](https://github.com/user-attachments/assets/7ee25c8c-8fe0-48e4-a3cc-6db4f1a8a750)


```bash
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 5000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```

