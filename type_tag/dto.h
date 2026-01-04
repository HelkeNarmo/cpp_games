#pragma once
#include <string>
#include <sstream>
#include <vector>
using namespace std;

struct PlayerDto {
    int id;
    int progress;
    
    // 序列化方法：将Player转换为字符串
    string serialize() const {
        ostringstream oss;
        oss << id << " " << progress;
        return oss.str();
    }
    
    // 反序列化方法：从字符串恢复Player
    static PlayerDto deserialize(const string& data) {
        PlayerDto player;
        istringstream iss(data);
        iss >> player.id >> player.progress;
        return player;
    }
};

// 序列化整个玩家列表（vector格式）
inline string serializeProgresses(const vector<int>& progresses) {
    ostringstream oss;
    oss << progresses.size() << " ";
    for (size_t i = 0; i < progresses.size(); ++i) {
        oss << i << " " << progresses[i];
        if (i < progresses.size() - 1) {
            oss << ";";
        }
    }
    return oss.str();
}

// 反序列化玩家列表到vector
inline vector<int> deserializeProgresses(const string& data) {
    vector<int> result;
    istringstream ss(data);
    string playerData;
    size_t size;
    ss >> size;
    result.resize(size);
    
    // 按分号分割每个玩家的数据
    while (getline(ss, playerData, ';')) {
        stringstream playerSs(playerData);
        int id, process;
        playerSs >> id >> process;
        if (id >= size) {
            continue;
        }
        result[id] = process;
    }
    return result;
}