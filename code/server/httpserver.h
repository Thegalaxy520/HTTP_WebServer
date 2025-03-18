//
// Created by moon on 25-3-14.
//

#ifndef HTTPSERVER_H
#define HTTPSERVER_H


#include <unordered_map>
#include <cstring>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../timer/heaptimer.h"
#include "../pool/threadpool.h"
#include "./epoller.h"



//服务器类
class HttpServer {
public:
	HttpServer(
		int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char* sqlUser, const  char* sqlPwd,
        const char* dbName, int connPoolNum, int threads,
        bool openLog, int logLevel, int logQueSize);

	 ~HttpServer();
    void Start();

private:



	static const int MAX_FD = 65536;

	static int SetFdNonblock(int fd);

	int port_;
	int openLinger_;
	int timeoutMS_;
	bool isClose_;
	int listenFd_;
	char* srcDir_;

	uint32_t listentEv_;
	uint32_t connEv_;

	std::unique_ptr<HeapTimer> timer_;
	std::unique_ptr<ThreadPool> threadPool;
	std::unique_ptr<Epoller> epoller;
};


#endif //HTTPSERVER_H
