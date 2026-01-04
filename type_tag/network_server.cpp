#include "network_server.h"

NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::setupRoutes() {
    server_->set_logger([](const httplib::Request&, const httplib::Response&) {});
    server_->Post("/text", [st = state_](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(st->mtx_);
        res.set_content(st->text, "text/plain; charset=utf-8");
    });
    server_->Post("/login", [st = state_](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lk(st->mtx_);
        int id = (int)st->progresses_.size();
        if (id >= st->maxPlayers) {
            res.set_content("-1", "text/plain");
            return;
        }
        st->progresses_.push_back(0);
        res.set_content(to_string(id), "text/plain");
    });
    server_->Post("/progress", [st = state_](const httplib::Request& req, httplib::Response& res) {
        PlayerDto p = PlayerDto::deserialize(req.body);
        {
            lock_guard<mutex> lk(st->mtx_);
            if (p.id >= 0 && p.id < (int)st->progresses_.size()) {
                st->progresses_[p.id] = p.progress;
            }
            res.set_content(serializeProgresses(st->progresses_), "text/plain");
        }
    });
}

bool NetworkServer::start(const string& bindAddr, int port, const string& text, int maxPlayers) {
    if (running_) return true;
    port_ = port;
    {
        lock_guard<mutex> lk(state_->mtx_);
        state_->text = text;
        state_->maxPlayers = maxPlayers;
        state_->progresses_.clear();
    }
    server_ = make_unique<httplib::Server>();
    setupRoutes();
    if (!server_->bind_to_port(bindAddr, port)) {
        cout << "Failed to bind to port " << port << endl;
        server_.reset();
        return false;
    }
    running_ = true;
    worker_ = thread([this]() {
        server_->listen_after_bind();
        running_ = false;
    });
    return true;
}

void NetworkServer::stop() {
    if (!server_) return;
    server_->stop();
    if (worker_.joinable()) worker_.join();
    server_.reset();
    running_ = false;
}