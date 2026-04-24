#pragma once

#include <string>

#include "channel.h"

class EventLoop;

class Dispatcher {
public:
    Dispatcher(EventLoop* evloop);
    virtual ~Dispatcher();
    //添加
    virtual int add(Channel* channel) = 0;
    //修改
    virtual int modify(Channel* channel) = 0;
    //删除
    virtual int remove(Channel* channel) = 0;
    //事件检测
    virtual void dispatch(int timeout = 2) = 0;

    inline std::string getName() {
        return m_name;
    }

protected:
    std::string m_name;
    EventLoop* m_evLoop;


};