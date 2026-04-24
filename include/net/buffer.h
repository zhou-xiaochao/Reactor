#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>

class Buffer {
public:
    Buffer();
    ~Buffer();
    //======读事件======
    //可读字节数
    int readableBytes() const;
    //获取读指针
    const char* peek() const;
    //读取len字节
    void retrieve(int len);
    //读取到指定位置
    void retrieveUntil(const char* end);
    //读取所有可读字节
    void retrieveAll();
    //读取未string
    std::string retrieveAsString(int len);
    //将所有可读取字节读为string
    std::string retrieveAllAsString();

    //======写事件======
    //可写字节数
    int writeableBytes() const;
    //获取写指针
    char* beginWrite();
    //移动写指针（已写入len字节）
    void hasWritten(int len);
    //追加数据
    void append(const char* data, int len);
    inline void append(const std::string& data) {
        append(data.c_str(), data.size());
    }
    //前置追加（协议头）
    void prepend(const char* data, int len);
    inline void prepend(const std::string& data) {
        prepend(data.c_str(), data.size());
    }

    int readFromFd(int cfd);

    int writeToFd(int cfd);

    const char* findEndSign() const;

    bool readLine(std::string& line);

    void setEndSign(std::string& flag);

private:
    void ensureWriteableBytes(int len);

private:
    std::vector<char> m_buffer;
    int m_readIndex;
    int m_writeIndex;
    char m_endSign[16];

};