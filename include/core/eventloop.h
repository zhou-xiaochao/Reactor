#pragma once

#include <unordered_map>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>

#include "dispatcher.h"

enum class Type {
    ADD,
    DELETE,
    MODIFY
};

struct Task {
    Type type;
    Channel* channel;
    Task() {}
    Task(Channel* ch, Type t) : channel(ch), type(t) {}
};

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void start();
    void stop();
    void addTask(Channel* channel, Type type);
    void EventActive(int fd, FdEvent event);
    static void disConnection(EventLoop* evloop);
    
private:
    void runTask();
    int typeAdd(Channel* channel);
    int typeDelete(Channel* channel);
    int typeModify(Channel* channel);
    void weakup();
    void handlerWeakup();

private:
    int m_weakupFd;
    Channel* m_weakupChannel;
    std::atomic<bool> m_isExit;
    Dispatcher* m_dispatcher;
    std::queue<std::unique_ptr<Task>> m_taskQ;
    std::unordered_map<int, Channel*> m_channelMap;
    std::mutex m_mtxTaskQ;

};