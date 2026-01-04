#pragma once

#include "animation.h"
#include "camera.h"

class Player {
public: 
    enum class Facing { DOWN = 0, UP = 1, LEFT = 2, RIGHT = 3 };

    static const int FRAME_W = 64;
    static const int FRAME_H = 64;

    Player() {}

    ~Player() = default;

    void init(SDL_Renderer* renderer, const string& idlePath, const string& walkPath) {
        animIdle.setAtlas(renderer, idlePath, FRAME_W, FRAME_H, 0, 0, 4, 6);
        animIdle.setInterval(0.1f);
        animIdle.setLoop(true);
        animWalk.setAtlas(renderer, walkPath, FRAME_W, FRAME_H, 0, 0, 4, 8);
        animWalk.setInterval(0.1f);
        animWalk.setLoop(true);
    }

    void onUpdate(float deltaTime) {
        if (!position.approx(posTarget)) {
            velocity = (posTarget - position).normalize() * SPEED_RUN;
        } else {
            velocity = Vector2(0.0f, 0.0f);
        }
        if ((posTarget - position).length() <= (velocity * deltaTime).length()) { // 本次更新会超过目标点
            position = posTarget;
        } else {
            position += velocity * deltaTime;
        }
        if (velocity.approx(Vector2(0.0f, 0.0f))) {
            curAnim = &animIdle;
        } else {
            if (abs(velocity.x) >= 0.0001f) {
                facing = velocity.x > 0.0f ? Facing::RIGHT : Facing::LEFT;
            } else if (abs(velocity.y) >= 0.0001f) {
                facing = velocity.y > 0.0f ? Facing::DOWN : Facing::UP;
            }
            curAnim = &animWalk;
        }
        if (!curAnim) return;
        curAnim->setRowFrame((int)facing);
        curAnim->setPosition(position);
        curAnim->onUpdate(deltaTime);
    }

    void onRender(SDL_Renderer *renderer, const Camera& camera) {
        if (!curAnim) return;
        curAnim->onRender(renderer, camera);
    }

    void setPosition(const Vector2& pos) {
        position = pos;
        posTarget = pos;
    }

    Vector2 getPosition() const {
        return position;
    }

    void setTarget(const Vector2& target) {
        posTarget = target;
    }

private:
    const float SPEED_RUN = 100.0f;

    Vector2 position;
    Vector2 velocity;
    Vector2 posTarget;

    Animation animIdle;
    Animation animWalk;
    Animation* curAnim = nullptr;

    Facing facing = Facing::DOWN;
};