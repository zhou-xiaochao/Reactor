#include <iostream>
#include <unistd.h>
#include <fstream>

#include "tcpserver.h"
#include "config.h"

using namespace std;

#define CONFIG_FILE "config.json"

const json default_cfg = {
    {"protocol_type", "http"},
    {"buffer_end_sign", "\r\n"},
    {"dispatcher_way", "epoll"}
};

bool createDefaultCfg() {
    ofstream ofs(CONFIG_FILE);
    if(!ofs.is_open()) {
        return false;
    }
    ofs << default_cfg.dump(4) << endl;
    ofs.close();
    return true;
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        cerr << "input invalid\n";
        return 0;
    }
    string dir = argv[2];
    chdir(dir.c_str());
    if(!Config::instance().load(CONFIG_FILE)) {
        if(createDefaultCfg()) {
            cerr << "failed to create config.json\n";
            return 0;
        }
    }
    unsigned short port = atoi(argv[1]);
    TcpServer* server = new TcpServer(port);
    server->start();
}