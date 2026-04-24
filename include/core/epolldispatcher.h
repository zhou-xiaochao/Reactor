#pragma once

#include <sys/epoll.h>

#include "dispatcher.h"
#include "eventloop.h"

class EpollDispatcher : public Dispatcher{
public:
    EpollDispatcher(EventLoop* evloop);
    ~EpollDispatcher();
    int add(Channel* channel) override;
    int remove(Channel* channel) override;
    int modify(Channel* channel) override;
    void dispatch(int timeout) override;

private:
    int epollCtl(int op, Channel* channel);

private:
    int m_epfd;
    epoll_event* m_events;

};