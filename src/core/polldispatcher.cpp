#include <iostream>

#include "polldispatcher.h"

using namespace std;

static const int PFDS_SIZE = 4096;

PollDispatcher::PollDispatcher(EventLoop *eventloop) : Dispatcher(eventloop)
{
    m_maxfd = 0;
    m_pfds = new pollfd[PFDS_SIZE];
    for(int i = 0; i < PFDS_SIZE; i ++) {
        m_pfds[i].fd = -1;
        m_pfds[i].events = 0;
        m_pfds[i].revents = 0;
    }
    m_name = "Poll";
}

PollDispatcher::~PollDispatcher()
{
    delete[] m_pfds;
}

int PollDispatcher::add(Channel *channel)
{
    int event = 0;
    if(channel->withReadEvent()) {
        event |= POLLIN;
    }
    if(channel->withWriteEvent()) {
        event |= POLLOUT;
    }
    for(int i = 0; i < PFDS_SIZE; i ++) {
        if(m_pfds[i].fd == -1) {
            m_pfds[i].fd = channel->getSocket();
            m_pfds[i].events = event;
            m_maxfd = m_maxfd < i ? i : m_maxfd;
            return 1;
        }
    }
    return -1;
}

int PollDispatcher::modify(Channel *channel)
{
    int event = 0;
    if(channel->withReadEvent()) {
        event |= POLLIN;
    }
    if(channel->withWriteEvent()) {
        event |= POLLOUT;
    }
    for(int i = 0; i < m_maxfd + 1; i ++) {
        if(m_pfds[i].fd == channel->getSocket()) {
            m_pfds[i].events = event;
            return 1;
        }
    }
    return -1;
}

int PollDispatcher::remove(Channel *channel)
{
    for(int i = 0; i < m_maxfd; i ++) {
        if(m_pfds[i].fd == channel->getSocket()) {
            m_pfds[i].fd = -1;
            m_pfds[i].events = 0;
            m_pfds[i].revents = 0;
            return 1;
        }
    }
    return -1;
}

void PollDispatcher::dispatch(int timeout)
{
    int count = poll(m_pfds, m_maxfd + 1, timeout * 1000);
    if(count == -1) {
        cerr << "poll failed\n";        
    }
    for(int i = 0; i < m_maxfd + 1; i ++) {
        if(m_pfds[i].fd == -1) continue;
        if(m_pfds[i].revents & POLLIN) {
            //处理读事件
            m_evLoop->EventActive(m_pfds[i].fd, FdEvent::ReadEvent);
        }
        if(m_pfds[i].revents & POLLOUT) {
            //处理写事件
            m_evLoop->EventActive(m_pfds[i].fd, FdEvent::WriteEvent);
        }
    }
}
