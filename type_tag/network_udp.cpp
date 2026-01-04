#include "network_udp.h"
#include <SDL2/SDL_net.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <functional>
#include <chrono>
#include <sstream>
#include <vector>
#include <cstring>

static bool ParseAnnounce(const string& msg, HostAnnounce& out) {
    if (msg.rfind("TYPETAG|1|") != 0) return false;
    vector<string> parts;
    {
        stringstream ss(msg);
        string item;
        while (getline(ss, item, '|')) parts.push_back(item);
    }
    if (parts.size() < 6) return false;
    try {
        out.httpPort = stoi(parts[2]);
        out.roomName = parts[3];
        out.playerCnt = stoi(parts[4]);
        out.maxPlayer = stoi(parts[5]);
    }
    catch(...) {
        return false;
    }
    return true;
}

// UDP广播实现
bool UDPBroadcaster::start(int udpPort, function<string()> payloadFn, int intervalMs) {
    if (running_) return true;

    running_ = true;
    th_ = thread([this, udpPort, payloadFn, intervalMs]() {
        UDPsocket sock = SDLNet_UDP_Open(0); // 绑定到任意可用端口
        if (!sock) {
            cout << "Failed to open UDP socket for broadcasting: " << SDLNet_GetError() << endl;
            running_ = false;
            return;
        }

        IPaddress broadcastAddr;
        if (SDLNet_ResolveHost(&broadcastAddr, "255.255.255.255", udpPort) == -1) {
            cout << "Failed to resolve broadcast address: " << SDLNet_GetError() << endl;
            SDLNet_UDP_Close(sock);
            running_ = false;
            return;
        }

        UDPpacket* packet = SDLNet_AllocPacket(1024);
        if (!packet) {
            cout << "Failed to allocate UDP packet: " << SDLNet_GetError() << endl;
            SDLNet_UDP_Close(sock);
            running_ = false;
            return;
        }

        while (running_) {
            string payload = payloadFn();
            if (payload.length() >= packet->maxlen) {
                cout << "Payload too large for UDP packet" << endl;
                continue;
            }

            memcpy(packet->data, payload.c_str(), payload.length());
            packet->len = payload.length();
            packet->address = broadcastAddr;

            if (SDLNet_UDP_Send(sock, -1, packet) == 0) {
                cout << "Failed to send UDP broadcast: " << SDLNet_GetError() << endl;
            }

            this_thread::sleep_for(chrono::milliseconds(intervalMs));
        }

        SDLNet_FreePacket(packet);
        SDLNet_UDP_Close(sock);
    });

    return true;
}

void UDPBroadcaster::stop() {
    if (!running_) return;
    running_ = false;
    if (th_.joinable()) th_.join();
}

// UDP监听实现
bool UDPListener::start(int udpPort, function<void(const HostAnnounce&)> onHost, int intervalMs) {
    if (running_) return true;

    running_ = true;
    th_ = thread([this, udpPort, onHost, intervalMs]() {
        UDPsocket sock = SDLNet_UDP_Open(udpPort);
        if (!sock) {
            cout << "Failed to open UDP socket for listening: " << SDLNet_GetError() << endl;
            running_ = false;
            return;
        }

        UDPpacket* packet = SDLNet_AllocPacket(1024);
        if (!packet) {
            cout << "Failed to allocate UDP packet: " << SDLNet_GetError() << endl;
            SDLNet_UDP_Close(sock);
            running_ = false;
            return;
        }

        while (running_) {
            int result = SDLNet_UDP_Recv(sock, packet);
            if (result > 0) {
                string msg((char*)packet->data, packet->len);

                // 获取发送者的IP地址
                Uint32 host = SDL_SwapBE32(packet->address.host);
                string ip = to_string((host >> 24) & 0xFF) + "." +
                           to_string((host >> 16) & 0xFF) + "." +
                           to_string((host >> 8) & 0xFF) + "." +
                           to_string(host & 0xFF);

                HostAnnounce announce;
                if (ParseAnnounce(msg, announce)) {
                    announce.ip = ip;
                    onHost(announce);
                }
            } else if (result == -1) {
                cout << "UDP receive error: " << SDLNet_GetError() << endl;
            }

            this_thread::sleep_for(chrono::milliseconds(intervalMs));
        }

        SDLNet_FreePacket(packet);
        SDLNet_UDP_Close(sock);
    });

    return true;
}

void UDPListener::stop() {
    if (!running_) return;
    running_ = false;
    if (th_.joinable()) th_.join();
}