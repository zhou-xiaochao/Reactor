#include <fstream>

#include "config.h"

using namespace std;

Config &Config::instance()
{
    static Config ins;
    return ins;
    // TODO: 在此处插入 return 语句
}

bool Config::load(const std::string &path)
{
    ifstream file(path);
    if (!file) return false;
    file >> m_json;
    return true;
}

std::string Config::getProtocolType() const
{
    return m_json["protocol_type"].get<string>();
}

std::string Config::getBufferEndSign() const
{
    return m_json["buffer_end_sign"].get<string>();
}

std::string Config::getDispatcherWay() const
{
    return m_json["dispatcher_way"].get<string>();
}
