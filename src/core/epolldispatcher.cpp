#include <iostream>
#include <unistd.h>

#include "epolldispatcher.h"

using namespace std;

static const int EVS_SIZE = 1024;

EpollDispatcher::EpollDispatcher(EventLoop *evloop) : Dispatcher(evloop)
{
    m_epfd = epoll_create(1);
    if(m_epfd == -1) {
        cerr << "epoll_create failed\n";
    }
    m_events = new epoll_event[EVS_SIZE];
    m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher()
{
    delete[] m_events;
    m_events = nullptr;
    close(m_epfd);
}

int EpollDispatcher::add(Channel* channel)
{
    int res = epollCtl(EPOLL_CTL_ADD, channel);
    if(res == -1) {
        cerr << "EPOLL_CTL_ADD failed\n";
    }
    return res;
}

int EpollDispatcher::remove(Channel* channel)
{
    int res = epollCtl(EPOLL_CTL_DEL, channel);
    if(res == -1) {
        cerr << "EPOLL_CTL_DEL failed\n";
    }
    return res;
}

int EpollDispatcher::modify(Channel* channel)
{
    int res = epollCtl(EPOLL_CTL_MOD, channel);
    if(res == -1) {
        cerr << "EPOLL_CTL_MOD failed\n";
    }
    return res;
}

void EpollDispatcher::dispatch(int timeout)
{
    int count = epoll_wait(m_epfd, m_events, EVS_SIZE, timeout * 1000);
    for(int i = 0; i < count; i ++) {
        int event = m_events[i].events;
        int fd = m_events[i].data.fd;
        if(event & EPOLLERR || event & EPOLLHUP) {
            continue;
        }
        if(event & EPOLLIN) {
            //EventLoop中处理读事件
            m_evLoop->EventActive(fd, FdEvent::ReadEvent);
        }
        if(event & EPOLLOUT) {
            //EventLoop中处理写事件
            m_evLoop->EventActive(fd, FdEvent::WriteEvent);
        }
    }
}

int EpollDispatcher::epollCtl(int op, Channel* channel)
{
    epoll_event ev;
    ev.events = 0;
    ev.data.fd = channel->getSocket();
    if(channel->withReadEvent()) {
        ev.events |= EPOLLIN;
    }
    if(channel->withWriteEvent()) {
        ev.events |= EPOLLOUT;
    }
    int res = epoll_ctl(m_epfd, op, channel->getSocket(), &ev);
    return res;
}
