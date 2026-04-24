#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>

#include "httpprotocol.h"
#include "buffer.h"
#include "tcpconnection.h"

using namespace std;

HttpProtocol::HttpProtocol()
{
    m_pareseResult = ParseResult::kIncomplete;
    m_currentState = ProcessState::ParseReqLine;
    m_isReadLineF = false;
}

bool HttpProtocol::ParseRequestLine(Buffer *readBuffer)
{
    string line;
    if(!readBuffer->readLine(line)) {
        m_isReadLineF = true;
        return false;
    }
    istringstream iss(line);
    iss >> m_method >> m_url >> m_version;
    if(m_method.empty() || m_url.empty() || m_version.empty()) {
        return false;
    }
    setCurrentState(ProcessState::ParseReqHeaders);
    return true;
}

bool HttpProtocol::ParseRequestHeader(Buffer *readBuffer)
{
    string line;
    if(!readBuffer->readLine(line)) {
        m_isReadLineF = true;
        return false;
    }
    int colon = line.find(':');
    if(colon == string::npos) {
        setCurrentState(ProcessState::ParseReqDone);
        return true;
    }
    
    string key = line.substr(0, colon);
    string value = line.substr(colon + 1);
    
    // 去除 value 前导空格
    int start = value.find_first_not_of(" \t");
    if(start != string::npos) {
        value = value.substr(start);
    }
    
    // 去除 value 尾部 \r
    if(!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
    
    m_reqHeaders.insert(make_pair(key, value));
    return true;
}

bool HttpProtocol::ParseRequestBody(Buffer *readBuffer)
{
    return false;
}

ParseResult HttpProtocol::tryParse(Buffer *input)
{
    bool flag = true;
    while(m_pareseResult != ParseResult::kComplete) {
        switch(m_currentState) {
            case ProcessState::ParseReqLine : {
                flag = ParseRequestLine(input);
                break;
            }
            case ProcessState::ParseReqHeaders : {
                flag = ParseRequestHeader(input);
                break;
            }
            case ProcessState::ParseReqBody : {
                flag = ParseRequestBody(input);
                break;
            }
            default :
                break;
        }
        if(!flag) {
            if(!m_isReadLineF) m_pareseResult = ParseResult::kError;
            else m_pareseResult = ParseResult::kIncomplete;
            m_isReadLineF = false;
            break;
        }
        if(m_currentState == ProcessState::ParseReqDone) {
            m_pareseResult = ParseResult::kComplete;
        }
    }
    return m_pareseResult;
}

void HttpProtocol::encode(TcpConnection* conn, Buffer *output)
{
    ostringstream tmp;
    tmp << m_version << " " << static_cast<int>(m_statusCode) << " " << m_info.at(m_statusCode) << "\r\n";
    output->append(tmp.str());
    for(auto it = m_headers.begin(); it != m_headers.end(); it ++) {
        tmp.str("");
        tmp << it->first << ": " << it->second << "\r\n";
        output->append(tmp.str());
    }
    output->append("\r\n");
    if(sendDataFunc != nullptr) sendDataFunc(conn, m_fileName, output);
    else {
        output->append("<h1>404 Not Found</h1>");
    }
}

void HttpProtocol::onMessage(TcpConnection *conn)
{
    if(m_method.compare("GET") != 0) return;
    m_url = decodeMsg(m_url);
    const char* file = NULL;
    if(m_url.compare("/") == 0) {
        file = "./";
    } else {
        file = m_url.data() + 1;
    }
    struct stat st;
    int ret = stat(file, &st);
    if(ret == -1) {
        cerr << file << '\n';
        cerr << "404 not found\n";
        setStatusCode(StatusCode::NotFound);
        addHeader("Content-type", getFileType(".html"));
        addHeader("Content-Length", "16");
        sendDataFunc = nullptr;
        conn->send();
        m_headers.clear();
        m_pareseResult = ParseResult::kIncomplete;
        m_currentState = ProcessState::ParseReqLine;
        return;
    }
    setFileName(file);
    setStatusCode(StatusCode::OK);
    if(S_ISDIR(st.st_mode)) {
        addHeader("Content-type", getFileType(".html"));
        sendDataFunc = [this](TcpConnection* conn, string name, Buffer* output) {
            sendDir(conn, name, output);
        };
        conn->send();
    } else {
        addHeader("Content-type", getFileType(file));
        addHeader("Content-length", to_string(st.st_size));
        sendDataFunc = [this](TcpConnection* conn, string name, Buffer* output) {
            sendFile(conn, name, output);
        };
        conn->send();
    }
    m_headers.clear();
    m_pareseResult = ParseResult::kIncomplete;
    m_currentState = ProcessState::ParseReqLine;
}

std::string HttpProtocol::name() const
{
    return "HTTP";
}

void HttpProtocol::sendFile(TcpConnection *conn, std::string name, Buffer *output)
{
    int fd = open(name.c_str(), O_RDONLY);
    struct stat st;
    stat(name.c_str(), &st);
    conn->sendFile(fd, st.st_size);
}

void HttpProtocol::sendDir(TcpConnection *conn, std::string name, Buffer *output)
{
    ostringstream oss;
    oss << "<html><head><title>" << name << "</title></head><body><table>";
    dirent** namelist = nullptr;
    int num = scandir(name.c_str(), &namelist, nullptr, alphasort);
    if(num < 0 || namelist == nullptr) {
        oss << "<tr><td>无法读取目录</td></tr>";
        oss << "</table></body></html>";
        output->append(oss.str());
        return;
    }
    for(int i = 0; i < num; i ++) {
        string dir = namelist[i]->d_name;
        struct stat st;
        ostringstream sub;
        sub << name << "/" << dir;
        if(stat(sub.str().c_str(), &st) == -1) {
            continue;
        }
        if(S_ISDIR(st.st_mode)) {
            oss << "<tr><td><a href=\"" << dir << "/\">" << dir << "</a></td><td>" << st.st_size << "</td></tr>";
        } else {
            oss << "<tr><td><a href=\"" << dir << "\">" << dir << "</a></td><td>" << st.st_size << "</td></tr>";
        }
        free(namelist[i]);
    }
    oss << "</table></body></html>";
    output->append(oss.str());
    free(namelist);
}

const std::string HttpProtocol::getFileType(std::string name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name.data(), '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".mp4") == 0 || strcmp(dot, ".m4v") == 0)
        return "video/mp4";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

std::string HttpProtocol::decodeMsg(std::string msg)
{
    string str = string();
    const char* from = msg.data();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }
    }
    // str.append(1, '\0');
    return str;
}

int HttpProtocol::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}
