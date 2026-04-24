#include "workthread.h"

using namespace std;

WorkThread::WorkThread() : m_thread(nullptr), m_evLoop(nullptr)
{
}

WorkThread::~WorkThread()
{
    if(m_thread->joinable()) m_thread->join();
    delete m_thread;
    delete m_evLoop;
}

void WorkThread::start()
{
    m_thread = new thread(&WorkThread::run, this);
    unique_lock<mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() {
        return m_evLoop != nullptr;
    });
}

void WorkThread::run()
{
    {
        unique_lock<mutex> lock(m_mutex);
        m_evLoop = new EventLoop();
    }
    m_cond.notify_one();
    m_evLoop->start();
}
