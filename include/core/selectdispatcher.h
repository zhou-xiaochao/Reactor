#pragma once

#include <sys/select.h>

#include "dispatcher.h"
#include "eventloop.h"

class SelectDispatcher : public Dispatcher {
public:
    SelectDispatcher(EventLoop* eventloop);
    ~SelectDispatcher();
    int add(Channel* channel) override;
    int modify(Channel* channel) override;
    int remove(Channel* channel) override;
    void dispatch(int timeout) override;

private:
    void setFdSet(Channel* channel);
    void clearFdSet(Channel* channel);

private:
    int m_maxfd;
    fd_set m_readSet;
    fd_set m_writeSet;

};