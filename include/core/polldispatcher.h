#pragma once

#include <poll.h>

#include "dispatcher.h"
#include "eventloop.h"

class PollDispatcher : public Dispatcher {
public:
    PollDispatcher(EventLoop* eventloop);
    ~PollDispatcher();
    int add(Channel* channel) override;
    int modify(Channel* channel) override;
    int remove(Channel* channel) override;
    void dispatch(int timeout) override;

private:
    int m_maxfd;
    pollfd* m_pfds;

}; 