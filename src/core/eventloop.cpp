#include <unistd.h>

#include "eventloop.h"
#include "epolldispatcher.h"
#include "selectdispatcher.h"
#include "polldispatcher.h"
#include "config.h"

using namespace std;

EventLoop::EventLoop()
{
    m_isExit.store(true);
    m_weakupFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    string way = Config::instance().getProtocolType();
    if(way == "poll") {
        m_dispatcher = new PollDispatcher(this);
    } else if(way == "select") {
        m_dispatcher = new SelectDispatcher(this);
    } else {
        m_dispatcher = new EpollDispatcher(this);
    }
    m_weakupChannel = new Channel(m_weakupFd, FdEvent::ReadEvent, [this]() {handlerWeakup();}, nullptr, nullptr);
    m_dispatcher->add(m_weakupChannel);
    m_channelMap.insert(make_pair(m_weakupFd, m_weakupChannel));
}

EventLoop::~EventLoop()
{
    for(auto it : m_channelMap) {
        if(it.first == m_weakupFd) continue;
        addTask(it.second, Type::DELETE);
    }
    m_isExit.store(true);
    m_dispatcher->remove(m_weakupChannel);
    m_channelMap.erase(m_weakupFd);
    delete m_weakupChannel;
    delete m_dispatcher;
}

void EventLoop::start()
{
    m_isExit.store(false);
    while(!m_isExit) {
        m_dispatcher->dispatch();
        runTask();
    }
}

void EventLoop::stop()
{
    m_isExit.store(true);
}

void EventLoop::addTask(Channel *channel, Type type)
{
    {
        lock_guard<mutex> lock(m_mtxTaskQ);
        m_taskQ.emplace(make_unique<Task>(channel, type));
    }
    weakup();
}

void EventLoop::EventActive(int fd, FdEvent event)
{
    Channel* channel = m_channelMap[fd];
    if(event == FdEvent::ReadEvent && channel->fun_callRead) {
        channel->fun_callRead();
    }
    if(event == FdEvent::WriteEvent && channel->fun_callWrite) {
        channel->fun_callWrite();
    }
}

void EventLoop::disConnection(EventLoop* evloop)
{
    close(evloop->m_weakupChannel->getSocket());
}

void EventLoop::runTask()
{
    while(true) {
        auto task = make_unique<Task>();
        {
            lock_guard<mutex> lock(m_mtxTaskQ);
            if(!m_taskQ.empty()) {
                task = move(m_taskQ.front());
                m_taskQ.pop();
            } else {
                break;
            }
        }
        if(task == nullptr) continue;
        if(task->type == Type::ADD) {
            typeAdd(task->channel);
        } else if(task->type == Type::DELETE) {
            typeDelete(task->channel);
        } else {
            typeModify(task->channel);
        }
    }
}

int EventLoop::typeAdd(Channel *channel)
{
    auto it = m_channelMap.find(channel->getSocket());
    if(it == m_channelMap.end()) {
        int res = m_dispatcher->add(channel);
        if(res != -1) {
            m_channelMap.insert(make_pair(channel->getSocket(), channel));
        }
        return res;
    }
    return -1;
}

int EventLoop::typeDelete(Channel *channel)
{
    auto it = m_channelMap.find(channel->getSocket());
    if(it != m_channelMap.end()) {
        int res = m_dispatcher->remove(channel);
        if(res != -1) {
            m_channelMap.erase(it);
            channel->fun_callDistory();
        }
        return res;
    }
    return -1;
}

int EventLoop::typeModify(Channel *channel)
{
    auto it = m_channelMap.find(channel->getSocket());
    if(it != m_channelMap.end()) {
        int res = m_dispatcher->modify(channel);
        return res;
    }
    return -1;
}

void EventLoop::weakup()
{
    uint64_t flag = 1;
    ssize_t n = write(m_weakupFd, &flag, sizeof(flag));
}

void EventLoop::handlerWeakup()
{
    uint64_t flag = 1;
    ssize_t n = read(m_weakupFd, &flag, sizeof(flag));
}
