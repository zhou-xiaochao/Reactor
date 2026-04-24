#include "buffer.h"

using namespace std;

static const int kInitialSize = 1024;
static const int kMaxBufferSize = 65536;

Buffer::Buffer() : m_buffer(kInitialSize), m_readIndex(0), m_writeIndex(0)
{
}

Buffer::~Buffer()
{
}

int Buffer::readableBytes() const
{
    return m_writeIndex - m_readIndex;
}

const char *Buffer::peek() const
{
    return m_buffer.data() + m_readIndex;
}

void Buffer::retrieve(int len)
{
    if(len < readableBytes()) {
        m_readIndex += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char *end)
{
    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    m_readIndex = 0;
    m_writeIndex = 0;
}

std::string Buffer::retrieveAsString(int len)
{
    string res(peek(), len);
    retrieve(len);
    return res;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

int Buffer::writeableBytes() const
{
    return m_buffer.size() - m_writeIndex;
}

char *Buffer::beginWrite()
{
    return m_buffer.data() + m_writeIndex;
}

void Buffer::hasWritten(int len)
{
    m_writeIndex += len;
}

void Buffer::append(const char *data, int len)
{
    ensureWriteableBytes(len);
    copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::prepend(const char *data, int len)
{
    if(m_readIndex >= len) {
        m_readIndex -= len;
        copy(data, data + len, m_buffer.data() + m_readIndex);
    } else {
        int val = readableBytes();
        vector<char> newBuf(val + len);
        copy(data, data + len, newBuf.data());
        copy(peek(), peek() + val, newBuf.data() + len);
        m_buffer.swap(newBuf);
        m_readIndex = 0;
        m_writeIndex = val + len;
    }
}

int Buffer::readFromFd(int cfd)
{
    char extrabuf[kMaxBufferSize];
    iovec vec[2];
    int val = writeableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = val;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    int n = readv(cfd, vec, 2);
    if(n < 0) {
        return n;
    } else if(n <= val) {
        hasWritten(n);
    } else {
        m_writeIndex = m_buffer.size();
        append(extrabuf, n - val);
    }
    return n;
}

int Buffer::writeToFd(int cfd)
{
    int n = write(cfd, peek(), readableBytes());
    if(n > 0) {
        retrieve(n);
    }
    return n;
}

const char *Buffer::findEndSign() const
{
    int val = readableBytes();
    const char* endSign = search(peek(), peek() + val, m_endSign, m_endSign + strlen(m_endSign));
    return endSign == m_buffer.data() + m_writeIndex ? nullptr : endSign;
}

bool Buffer::readLine(std::string &line)
{
    const char* endSing = findEndSign();
    if(endSing) {
        line.assign(peek(), endSing - peek());
        retrieve(endSing - peek() + strlen(m_endSign));
        return true;
    }
    return false;
}

void Buffer::setEndSign(std::string &flag)
{
    strncpy(m_endSign, flag.c_str(), 15);
    m_endSign[15] = '\0';
}

void Buffer::ensureWriteableBytes(int len)
{
    if(writeableBytes() >= len) {
        return;
    }
    if(m_readIndex + writeableBytes() >= len) {
        if(m_readIndex > 0) {
            int val = readableBytes();
            copy(peek(), peek() + val, m_buffer.data());
            m_readIndex = 0;
            m_writeIndex = val;
        }
    } else {
        int newSize = m_writeIndex + len;
        if(newSize > kMaxBufferSize) {
            newSize = kMaxBufferSize;
        }
        m_buffer.resize(newSize);
    }
}
