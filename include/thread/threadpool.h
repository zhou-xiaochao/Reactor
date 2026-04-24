#pragma once

#include <thread>
#include <vector>

#include "workthread.h"
#include "eventloop.h"

class ThreadPool {
public:
    ThreadPool(EventLoop* elp, int num);
    ~ThreadPool();
    void start();
    EventLoop* takeWorkerEventLoop();

private:
    int m_threadNum;
    EventLoop* m_evLoop;
    std::vector<WorkThread*> m_workThreads;
    int m_indx;

};