#pragma once

#include <functional>

//事件枚举类型
enum class FdEvent {
    ReadEvent = 0x01,
    WriteEvent = 0x02
};

inline FdEvent operator|(FdEvent x, FdEvent y) {
    return static_cast<FdEvent>(static_cast<int>(x) | static_cast<int>(y));
}

inline FdEvent operator&(FdEvent x, FdEvent y) {
    return static_cast<FdEvent>(static_cast<int>(x) & static_cast<int>(y));
}

inline FdEvent operator~(FdEvent x) {
    return static_cast<FdEvent>(~static_cast<int>(x) & static_cast<int>(FdEvent::ReadEvent | FdEvent::WriteEvent));
}

inline FdEvent& operator|=(FdEvent& x, FdEvent y) {
    return x = x | y;
}

inline FdEvent& operator&=(FdEvent& x, FdEvent y) {
    return x = x & y;
}

class Channel {
public:
    using CallBack = std::function<void()>;
    //读回调
    CallBack fun_callRead;
    //写回调
    CallBack fun_callWrite;
    //销毁回调
    CallBack fun_callDistory;
    Channel(int fd, FdEvent events, CallBack readFun, CallBack writeFun, CallBack distoryFun);
    ~Channel() = default;
    inline int getSocket() {
        return m_fd;
    }
    void enableReadEvent(bool flag);
    void enableWriteEvent(bool flag);
    bool withReadEvent();
    bool withWriteEvent();

private:
    //文件描述符
    int m_fd;
    //要监听的事件
    FdEvent m_fdEvent;

};