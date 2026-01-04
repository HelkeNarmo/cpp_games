#include "network_client.h"
#include "dto.h"

NetworkClient::~NetworkClient() {
    stop();
}

bool NetworkClient::connectAndLogin(const string& addr, int port, int& outId) {
    host_ = addr;
    port_ = port;
    client_ = make_unique<httplib::Client>(addr, port);
    client_->set_keep_alive(true);
    httplib::Result res = client_->Post("/login");
    if (!res) {
        cout << "login failed: no response" << endl;
        return false; 
    }
    if (res->status != 200) {
        cout << "login failed: status " << res->status << endl;
        return false; 
    }
    int id = -1;
    try {
        id = stoi(res->body);
    } catch (...) {
        cout << "login failed: invalid id" << endl;
        return false;
    }
    if (id < 0) {
        cout << "login failed: server is full" << endl;
        return false;
    }
    {
        lock_guard<mutex> lk(mtx_);
        me_.id = id;
        me_.progress = 0;
    }
    outId = id;
    return true;
}

bool NetworkClient::fetchText(string& outText) {
    if (!client_) {
        cout << "fetchText failed: not connected" << endl;
        return false;
    }
    auto res = client_->Post("/text");
    if (!res) {
        cout << "fetchText failed: no response" << endl;
        return false;
    }
    if (res->status != 200) {
        cout << "fetchText failed: status " << res->status << endl;
        return false;
    }
    outText = res->body;
    return true;
}

void NetworkClient::submitProgress(int progress) {
    lock_guard<mutex> lk(mtx_);
    me_.progress = progress;
}

vector<int> NetworkClient::getLatestProgress() const {
    lock_guard<mutex> lk(mtx_);
    return progresses_;
}

void NetworkClient::startProgressLoop(chrono::milliseconds interval) {
    if (running_) return;
    running_ = true;
    worker_ = thread([this, interval]() {
        while (running_) {
            postProgressOnce();
            this_thread::sleep_for(interval);
        }
    });
}

bool NetworkClient::postProgressOnce() {
    if (!client_) {
        cout << "postProgressOnce failed: not connected" << endl;
        return false;
    }
    PlayerDto snapshot;
    {
        lock_guard<mutex> lk(mtx_);
        snapshot = me_;
    }
    auto res = client_->Post("/progress", snapshot.serialize(), "text/plain");
    if (!res) {
        cout << "postProgressOnce failed: no response" << endl;
        return false;
    }
    if (res->status != 200) {
        cout << "postProgressOnce failed: status " << res->status << endl;
        return false;
    }
    auto newProgresses = deserializeProgresses(res->body);
    {
        lock_guard<mutex> lk(mtx_);
        progresses_ = move(newProgresses);
    }
    return true;
}

void NetworkClient::stop() {
    if (!running_) return;
    running_ = false;
    if (worker_.joinable()) worker_.join();
    client_.reset();
}