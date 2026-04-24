#include <arpa/inet.h>
#include <iostream>

#include "tcpserver.h"
#include "tcpconnection.h"

using namespace std;

TcpServer::TcpServer(unsigned short port, int num) : m_port(port)
{
    m_evLoop = new EventLoop();
    m_threadPool = new ThreadPool(m_evLoop, num);
    setListen();
}

TcpServer::~TcpServer()
{
    delete m_evLoop;
    delete m_threadPool;
    delete m_channel;
}

void TcpServer::start()
{
    m_threadPool->start();
    m_evLoop->addTask(m_channel, Type::ADD);
    m_evLoop->start();
}

void TcpServer::acceptConnection(TcpServer* server)
{
    int cfd = accept(server->m_channel->getSocket(), NULL, NULL);
    EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();
    new TcpConnection(cfd, evLoop);
}

void TcpServer::disConnection(TcpServer *server)
{
    close(server->m_channel->getSocket());
}

void TcpServer::setListen()
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1) {
        cerr << "socket failed\n";
        return;
    }
    int opt = 1;
    int res = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(res == -1) {
        cerr << "setsockopt failed\n";
        return;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    res = bind(lfd, (sockaddr*)&addr, sizeof(addr));
    if(res == -1) {
        cerr << "bind failed\n";
        return;
    }
    res = listen(lfd, 128);
    if(res == -1) {
        cerr << "listen failed\n";
        return;
    }
    m_channel = new Channel(lfd, FdEvent::ReadEvent, [this]() {acceptConnection(this);}, nullptr, [this]() {disConnection(this);});
}
