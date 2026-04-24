#include <fcntl.h>
#include <iostream> 
#include <string>
#include <sys/sendfile.h>

#include "tcpconnection.h"
#include "httpprotocol.h"
#include "config.h"

using namespace std;

TcpConnection::TcpConnection(int cfd, EventLoop *evloop) 
: m_evLoop(evloop), m_readBuffer(nullptr), m_writeBuffer(nullptr), m_protocol(nullptr), m_sendFileFd(-1), m_sendFileSize(0)
{
    string endSing = Config::instance().getBufferEndSign();
    m_readBuffer = new Buffer();
    m_writeBuffer = new Buffer();
    m_readBuffer->setEndSign(endSing);
    m_writeBuffer->setEndSign(endSing);
    fcntl(cfd, F_SETFL, O_NONBLOCK);
    std::string protoType = Config::instance().getProtocolType();
    if (protoType == "http") {
        m_protocol = new HttpProtocol();
    }
    m_channel = new Channel(cfd, FdEvent::ReadEvent, [this]() {readCall(this);}, [this]() {writeCall(this);}, [this]() {distoryCall(this);});
    m_evLoop->addTask(m_channel, Type::ADD);
}

TcpConnection::~TcpConnection()
{
    delete m_readBuffer;
    delete m_writeBuffer;
    delete m_channel;
    delete m_protocol;
}

void TcpConnection::send()
{
    m_protocol->encode(this, m_writeBuffer);
    if(!m_channel->withWriteEvent()) {
        m_channel->enableWriteEvent(true);
        m_evLoop->addTask(m_channel, Type::MODIFY);
    }
}

void TcpConnection::sendFile(int fileFd, off_t fileSize)
{
    m_sendFileFd = fileFd;
    m_sendFileSize = fileSize;
    m_sendFileOffset = 0;
}

void TcpConnection::readCall(TcpConnection* conn)
{
    int n = conn->m_readBuffer->readFromFd(conn->m_channel->getSocket());
    if(n > 0) {
        while(conn->m_protocol) {
            ParseResult result = conn->m_protocol->tryParse(conn->m_readBuffer); 
            if(result == ParseResult::kComplete) {
                conn->m_protocol->onMessage(conn);
            } else if(result == ParseResult::kIncomplete) {
                break;
            } else {
                conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
                return;
            }
        }
    } else if(n == 0) {
        conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
        return;
    } else {
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
            return;
        }
    }
}

void TcpConnection::writeCall(TcpConnection* conn)
{
    if(conn->m_writeBuffer->readableBytes() > 0) {
        int n = conn->m_writeBuffer->writeToFd(conn->m_channel->getSocket());
        if(n > 0) {
            if(conn->m_writeBuffer->readableBytes() > 0) return;
        } else if(n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            //发送信息出错
            conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
            return;
        }
    }

    // 2. 发送文件（正确非阻塞版本）
    if (conn->m_sendFileFd != -1) {
        off_t remaining = conn->m_sendFileSize - conn->m_sendFileOffset;
        if (remaining <= 0) {
            // 文件发完了 → 关闭文件描述符
            close(conn->m_sendFileFd);
            conn->m_sendFileFd = -1;
        } else {
            int n = sendfile(conn->m_channel->getSocket(),
                                conn->m_sendFileFd,
                                &conn->m_sendFileOffset,
                                remaining);
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // 内核缓冲区满 → 必须保留写事件，下次继续发
                return;
            } else if (n < 0) {
                // 真正错误
                close(conn->m_sendFileFd);
                conn->m_sendFileFd = -1;
                conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
                return;
            }
            // n>0 发送成功一部分，继续走逻辑
        }
    }
    if(conn->m_writeBuffer->readableBytes() == 0 && conn->m_sendFileFd == -1) {
        conn->m_channel->enableWriteEvent(false);
        conn->m_evLoop->addTask(conn->m_channel, Type::MODIFY);
    }
    if(conn->m_protocol && !conn->m_protocol->keepAlive()) {
        conn->m_evLoop->addTask(conn->m_channel, Type::DELETE);
    }
}

void TcpConnection::distoryCall(TcpConnection* conn)
{
    if(conn->m_protocol) {
        conn->m_protocol->onDisconnected(conn);
    }
    if(conn->m_sendFileFd != -1) {
        close(conn->m_sendFileFd);
        conn->m_sendFileFd = -1;
    }
    close(conn->m_channel->getSocket());
    delete conn;
}
