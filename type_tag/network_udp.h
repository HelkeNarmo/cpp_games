#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <functional>

using namespace std;

struct HostAnnounce {
    string ip;
    int httpPort = 25565;
    string roomName;
    int playerCnt = 0;
    int maxPlayer = 0;
};

class UDPBroadcaster {
public:
    ~UDPBroadcaster() { stop(); }

    bool start(int udpPort, function<string()> playloadFn, int intervalMs = 1000);

    void stop();

private:
    atomic<bool> running_{false};
    thread th_;
};

class UDPListener {
public:
    ~UDPListener() { stop(); }

    bool start(int udpPort, function<void(const HostAnnounce&)> onHost, int intervalMs = 30);

    void stop();

private:
    atomic<bool> running_{false};
    thread th_;
};