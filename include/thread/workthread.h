#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include "eventloop.h"

class WorkThread {
public:
    WorkThread();
    ~WorkThread();
    void start();
    inline EventLoop* getEventLoop() {
        return m_evLoop;
    }

private:
    void run();

private:
    EventLoop* m_evLoop;
    std::thread* m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;

};