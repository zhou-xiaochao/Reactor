#pragma once

#include <thread>

#include "eventloop.h"
#include "threadpool.h"

class TcpServer {
public:
    TcpServer(unsigned short port, int num = std::thread::hardware_concurrency());
    ~TcpServer();
    void start();
    static void acceptConnection(TcpServer* server);
    static void disConnection(TcpServer* server);

private:
    void setListen();

private:
    int threadNum;
    EventLoop* m_evLoop;
    Channel* m_channel;
    unsigned short m_port;
    ThreadPool* m_threadPool;

};