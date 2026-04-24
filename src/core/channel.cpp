#include <unistd.h>

#include "channel.h"

Channel::Channel(int fd, FdEvent events, CallBack readFun, CallBack writeFun, CallBack distoryFun)
 : m_fd(fd), m_fdEvent(events), fun_callRead(readFun), fun_callWrite(writeFun), fun_callDistory(distoryFun) {
}

void Channel::enableReadEvent(bool flag)
{
    if(flag) {
        m_fdEvent |= FdEvent::ReadEvent;
    } else {
        m_fdEvent &= ~FdEvent::ReadEvent;
    }
}

void Channel::enableWriteEvent(bool flag)
{
    if(flag) {
        m_fdEvent |= FdEvent::WriteEvent;
    } else {
        m_fdEvent &= ~FdEvent::WriteEvent;
    }
}

bool Channel::withReadEvent()
{
    return (static_cast<int>(m_fdEvent & FdEvent::ReadEvent)) > 0 ? true : false;
}

bool Channel::withWriteEvent()
{
    return (static_cast<int>(m_fdEvent & FdEvent::WriteEvent)) > 0 ? true : false;
}
