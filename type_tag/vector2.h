#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>

class Vector2 {
public:
    float x = 0, y = 0;

    Vector2() = default;
    ~Vector2() = default;

    Vector2(float x, float y) : x(x), y(y) {}

    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    void operator+=(const Vector2& other) {
        x += other.x, y += other.y;
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    void operator-=(const Vector2& other) {
        x -= other.x, y -= other.y;
    }

    float operator*(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    void operator*=(float scalar) {
        x *= scalar, y *= scalar;
    }

    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    void operator/=(float scalar) {
        x /= scalar, y /= scalar;
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    Vector2 normalize() const {
        float len = length();
        if (len == 0) return Vector2(0, 0);
        return Vector2(x / len, y / len);
    }

    bool approx(const Vector2& target) {
        return (*this - target).length() < 0.0001f;
    }
};

#endif // !_VECTOR2_H_