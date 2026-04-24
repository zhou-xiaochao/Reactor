#pragma once

#include <memory>

#include "eventloop.h"
#include "protocol.h"
#include "buffer.h"

class TcpConnection {
public:
    TcpConnection(int cfd, EventLoop* evloop);
    ~TcpConnection();

    inline int getFd() {
        return m_channel->getSocket();
    }

    // 获取读写 Buffer（供 Protocol 使用）
    Buffer* readBuffer() { return m_readBuffer; }
    Buffer* writeBuffer() { return m_writeBuffer; }
    void send();
    void sendFile(int fileFd, off_t fileSize);

    static void readCall(TcpConnection* conn);
    static void writeCall(TcpConnection* conn);
    static void distoryCall(TcpConnection* conn);

private:
    EventLoop* m_evLoop;
    Buffer* m_readBuffer;
    Buffer* m_writeBuffer;
    Channel* m_channel;
    Protocol* m_protocol;
    int m_sendFileFd;
    off_t m_sendFileSize;
    off_t m_sendFileOffset;

};