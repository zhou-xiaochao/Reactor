#include "threadpool.h"

ThreadPool::ThreadPool(EventLoop *elp, int num) : m_evLoop(elp), m_threadNum(num), m_indx(0)
{
}

ThreadPool::~ThreadPool()
{
    for(auto it : m_workThreads) {
        delete it;
    }
}

void ThreadPool::start()
{
    for(int i = 0; i < m_threadNum; i ++) {
        WorkThread* wt = new WorkThread();
        wt->start();
        m_workThreads.push_back(wt);
    }
}

EventLoop *ThreadPool::takeWorkerEventLoop()
{
    EventLoop* evLoop = m_evLoop;
    evLoop = m_workThreads[m_indx]->getEventLoop();
    m_indx = ++m_indx % m_threadNum;
    return evLoop;
}
