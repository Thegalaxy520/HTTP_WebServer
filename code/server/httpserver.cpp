//
// Created by moon on 25-3-14.
//

#include "httpserver.h"

HttpServer::HttpServer(int port, int trigMode, int timeoutMs, bool OptLinger,
                       int sqlPort, const char *sqlUser, const char *sqlPwd,
                       const char *dbName, int connPoolNum, int threads, bool openLog, int logLevel, int logQueSize)
	: port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMs), isClose_(false),
	  timer_(new HeapTimer()), threadPool(new ThreadPool(threads)),epoller(new Epoller())
{
	srcDir_ = getcwd(nullptr, 256);
	assert(srcDir_);
	strncat(srcDir_, "/resources/", 16);
	HttpConn::userCount = 0;
	HttpConn::srcDir = srcDir_;
	SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

	InitEventMode_(trigMode);
	if (!InitSocket_()) { isClose_ = true; }

	if (openLog)
	{
		Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
		if (isClose_) { LOG_ERROR("========== Server init error!=========="); }
		else
		{
			LOG_INFO("========== Server init ==========");
			LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
			LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
			         (listenEvent_ & EPOLLET ? "ET" : "LT"),
			         (connEvent_ & EPOLLET ? "ET" : "LT"));
			LOG_INFO("LogSys level: %d", logLevel);
			LOG_INFO("srcDir: %s", HttpConn::srcDir);
			LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
		}
	}
}

HttpServer::~HttpServer()
{
}

void HttpServer::Start()
{
}

bool HttpServer::InitSocket_()
{
}
