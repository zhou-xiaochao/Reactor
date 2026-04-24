#pragma once

#include <functional>
#include <memory>
#include <string>

class Buffer;
class TcpConnection;

// 协议解析结果
enum class ParseResult {
    kComplete,   // 解析完成一条完整消息
    kIncomplete, // 数据不完整，需要继续读取
    kError       // 解析错误，应关闭连接
};

// 协议接口（抽象基类）
class Protocol {
public:
    virtual ~Protocol() = default;
    
    // ========== 核心解析接口 ==========
    
    // 从 Buffer 中尝试解析出一条完整消息
    // 返回值：
    //   - kComplete: 解析成功，output 中存放解析后的数据
    //   - kIncomplete: 数据不够，等待下次读取
    //   - kError: 协议错误
    virtual ParseResult tryParse(Buffer* input) = 0;
    
    // ========== 编码接口 ==========
    
    // 将消息编码为协议格式，写入 output Buffer
    virtual void encode(TcpConnection* conn, Buffer* output) = 0;
    
    // ========== 生命周期回调 ==========
    
    // 连接建立时调用
    virtual void onConnected(TcpConnection* conn) {}
    
    // 收到完整消息时调用（由 TcpConnection 触发）
    virtual void onMessage(TcpConnection* conn) = 0;
    
    // 连接关闭时调用
    virtual void onDisconnected(TcpConnection* conn) {}
    
    // ========== 辅助接口 ==========
    
    // 获取协议名称
    virtual std::string name() const = 0;
    
    // 是否保持连接（HTTP Keep-Alive 等）
    virtual bool keepAlive() const { return true; }

};