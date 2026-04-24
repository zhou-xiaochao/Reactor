#pragma once

#include "protocol.h"

enum class ProcessState:char
{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};

enum class StatusCode
{
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};

class HttpProtocol : public Protocol {
public:
    HttpProtocol();
    ~HttpProtocol() = default;
    // ========== 核心解析接口 ==========
    
    // 从 Buffer 中尝试解析出一条完整消息
    // 返回值：
    //   - kComplete: 解析成功，output 中存放解析后的数据
    //   - kIncomplete: 数据不够，等待下次读取
    //   - kError: 协议错误
    bool ParseRequestLine(Buffer *readBuffer);

    bool ParseRequestHeader(Buffer *readBuffer);

    bool ParseRequestBody(Buffer *readBuffer);

    ParseResult tryParse(Buffer* input) override;
    
    // ========== 编码接口 ==========
    
    // 将消息编码为协议格式，写入 output Buffer
    void encode(TcpConnection* conn, Buffer* output) override;
    
    // ========== 生命周期回调 ==========
    
    // 收到完整消息时调用（由 TcpConnection 触发）
    void onMessage(TcpConnection* conn) override;
    
    // ========== 辅助接口 ==========
    
    // 获取协议名称
    std::string name() const;
    
    // 是否保持连接（HTTP Keep-Alive 等）
    bool keepAlive() const { return true; }

private:
    inline void setCurrentState(ProcessState state) {
        m_currentState = state;
    }
    inline void setFileName(std::string filename) {
        m_fileName = filename;
    }
    inline void setStatusCode(StatusCode statu) {
        m_statusCode = statu;
    }
    inline void addHeader(const std::string& key, const std::string& value) {
        if(key.empty() || value.empty()) {
            return;
        }
        m_headers.insert(std::make_pair(key, value));
    }
    void sendFile(TcpConnection *conn, std::string name, Buffer* output);
    void sendDir(TcpConnection *conn, std::string name, Buffer* output);
    const std::string getFileType(std::string name);
    std::string decodeMsg(std::string msg);
    int hexToDec(char c);

private:
    ParseResult m_pareseResult;
    ProcessState m_currentState;
    std::string m_method;
    std::string m_url;
    std::string m_version;
    std::unordered_map<std::string, std::string> m_reqHeaders;
    bool m_isReadLineF;

    std::function<void(TcpConnection*, std::string, Buffer*)> sendDataFunc;

    StatusCode m_statusCode;
    std::string m_fileName;
    std::unordered_map<std::string, std::string> m_headers;
    const std::unordered_map<StatusCode, std::string> m_info = {
        {StatusCode::OK, "OK"},
        {StatusCode::MovedPermanently, "MovedPermanently"},
        {StatusCode::MovedTemporarily, "MovedTemporarily"},
        {StatusCode::BadRequest, "BadRequest"},
        {StatusCode::NotFound, "NotFound"}
    };
};