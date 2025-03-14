//
// Created by moon on 25-3-14.
//

#ifndef HTTPSERVER_H
#define HTTPSERVER_H


#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//服务器类
class HttpServer {
public:
	HttpServer(
		int port,int trigMode,int timeoutMs, bool OptLinger,
		int sqlPort,const char * sqlUser,const char * sqlPwd,
		const char * dbName,int connPoolNum,int threads,
		bool openLog,int logLevel,int logQueSize
	);

	~HttpServer();
	void Start();
private:
	bool InitSocket_();
};


#endif //HTTPSERVER_H
