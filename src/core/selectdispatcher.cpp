#include <iostream>

#include "selectdispatcher.h"

using namespace std;

static const int FDSET_SIZE = 1024;

SelectDispatcher::SelectDispatcher(EventLoop *eventloop) : Dispatcher(eventloop)
{
    m_maxfd = 0;
    FD_ZERO(&m_readSet);
    FD_ZERO(&m_writeSet);
    m_name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add(Channel* channel)
{
    if(channel->getSocket() >= FDSET_SIZE) {
        return -1;
    }
    setFdSet(channel);
    m_maxfd = m_maxfd < channel->getSocket() ? channel->getSocket() : m_maxfd;
    return 1;
}

int SelectDispatcher::modify(Channel* channel)
{
    setFdSet(channel);
    return 1;
}

int SelectDispatcher::remove(Channel* channel)
{
    clearFdSet(channel);
    return 1;
}

void SelectDispatcher::dispatch(int timeout)
{
    timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set rtmp = m_readSet;
    fd_set wtmp = m_writeSet;
    int count = select(m_maxfd + 1, &rtmp, &wtmp, NULL, &val);
    if(count == -1) {
        cerr << "select failed\n";
        return;
    }
    for(int i = 0; i < FDSET_SIZE; i ++) {
        if(FD_ISSET(i, &rtmp)) {
            //EventLoop中处理读事件            
            m_evLoop->EventActive(i, FdEvent::ReadEvent);
        }
        if(FD_ISSET(i, &wtmp)) {
            //EventLoop中处理写事件            
            m_evLoop->EventActive(i, FdEvent::WriteEvent);
        }
    }
}

void SelectDispatcher::setFdSet(Channel* channel)
{
    if(channel->withReadEvent()) {
        FD_SET(channel->getSocket(), &m_readSet);
    } else {
        FD_CLR(channel->getSocket(), &m_readSet);
    }
    if(channel->withWriteEvent()) {
        FD_SET(channel->getSocket(), &m_writeSet);
    } else {
        FD_CLR(channel->getSocket(), &m_writeSet);
    }
}

void SelectDispatcher::clearFdSet(Channel* channel)
{
    if(channel->withReadEvent()) {
        FD_CLR(channel->getSocket(), &m_readSet);
    }
    if(channel->withWriteEvent()) {
        FD_CLR(channel->getSocket(), &m_writeSet);
    }
}
