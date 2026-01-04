#pragma once

#include <functional>

class Timer {
public:
    Timer() = default;
    ~Timer() = default;

    void restart() {
        passTime = 0;
        shotted = false;
    }

    void setWaitTime(float waitTime) {
        this->waitTime = waitTime;
    }

    void setOneShot(bool oneShot) {
        this->oneShot = oneShot;
    }
    
    void setOnTimeout(std::function<void()> onTimeout) {
        this->onTimeout = onTimeout;
    }

    void pause() {
        paused = true;
    }

    void resume() {
        paused = false;
    }

    void onUpdate(float deltaTime) {
        if (paused) return;
        passTime += deltaTime;
        if (passTime >= waitTime) {
            bool canShot = (!oneShot || !shotted);
            shotted = true;
            if (canShot && onTimeout) {
                onTimeout();
            }
            passTime -= waitTime;
        }
    }
private:
    float passTime = 0.0f;
    float waitTime = 0.0f;
    bool oneShot = false;
    bool shotted = false;
    bool paused = false;
    std::function<void()> onTimeout = nullptr;
};