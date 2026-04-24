#pragma once
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Config {
public:
    static Config& instance();

    // 加载配置文件
    bool load(const std::string& path);

    // 给你用的配置项
    std::string getProtocolType() const;
    std::string getBufferEndSign() const;
    std::string getDispatcherWay() const;

private:
    Config() = default;
    json m_json;
};