#pragma once

#include "vector2.h"

class Camera {
public:
    Camera() = default;
    ~Camera() = default;

    void setMaxSize(int maxW, int maxH) {
        this->maxW = maxW;
        this->maxH = maxH;
    }

    void setSize(const Vector2 size) {
        this->size = size;
    }

    const Vector2& getSize() const {
        return size;
    }

    void setPosition(const Vector2 position) {
        this->position = position;
    }

    const Vector2& getPosition() const {
        return position;
    }

    void lookAt(const Vector2& target) {
        position = target - size / 2.0f;
        position.x = max(0.0f, min(position.x, (float)maxW - size.x));
        position.y = max(0.0f, min(position.y, (float)maxH - size.y));
    }
private:
    Vector2 size;
    Vector2 position;
    int maxW, maxH;
};