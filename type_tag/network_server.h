#pragma once
#include "./libs/httplib.h"
#include "dto.h"
#include <string>
#include <vector>

using namespace std;

struct GameState {
    mutex mtx_;
    string text;
    int maxPlayers = 2;
    vector<int> progresses_;
};

class NetworkServer {
public:
    NetworkServer() = default;
    ~NetworkServer();

    bool start(const string& bindAddr, int port, const string& text, int maxPlayers);
    void stop();
    bool isRunning() const { return running_; }
    int port() const { return port_; }
private:
    void setupRoutes();
private:
    shared_ptr<GameState> state_ = make_shared<GameState>();
    unique_ptr<httplib::Server> server_;
    thread worker_;
    atomic<bool> running_{false};
    int port_ = 0;
};