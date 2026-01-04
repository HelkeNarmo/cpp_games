#pragma once
#include "./libs/httplib.h"
#include "dto.h"
#include <string>
#include <vector>

using namespace std;

class NetworkClient {
public:
    NetworkClient() = default;
    ~NetworkClient();

    bool connectAndLogin(const string& addr, int port, int& outId);

    bool fetchText(string& outText);

    void startProgressLoop(chrono::milliseconds interval = chrono::milliseconds(100));

    void submitProgress(int progress);

    vector<int> getLatestProgress() const;

    void stop();
private:
    bool postProgressOnce();

    unique_ptr<httplib::Client> client_;
    string host_;
    int port_;

    mutable mutex mtx_;
    PlayerDto me_;
    vector<int> progresses_;
    atomic<bool> running_{false};
    thread worker_;
};