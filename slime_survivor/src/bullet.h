#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include "resource_cache.h"
#include "character.h"
using namespace std;

struct BulletAtlas {
    shared_ptr<SDL_Texture> texture;
    shared_ptr<const AtlasIndex> atlasIndex;
};

class Guardian {
public:
    static const int SIZE = 11;
    static const int START_X = 123;
    static const int START_Y = 269;
    static const int FRAME_NUM = 8;
    static const SDL_Point PIVOT;
    static const char* ATLAS_PATH;
    static const int RADIUS_MAX = 100;
    static const int RADIUS_MIN = 20;

    bool init(SDL_Renderer *r);
    void update(int dt_ms);
    void render();
    void setPos(int x, int y) { center_.x = x; center_.y = y; }
    void setTarget(const Character *t) { target_ = t; }
    void setOrbitDeg(float deg) { orbitDeg_ = deg; }
    void checkCollision(Enemy &enemy);

protected:
    SDL_Renderer *renderer_ = nullptr;
    BulletAtlas bulletAtlas_;
    float radius_ = RADIUS_MIN; // 旋转半径
    int orbitSpeedDegPerSec_ = 100;
    int spinSpeedDegPerSec_ = 500;
    int radiusSpeedPerSec_ = 20;
    float orbitDeg_ = 0; // 公转角度，向上为0度
    float spinDeg_ = 0;
    int frameIndex = 0;
    SDL_Rect dstRect_{0, 0, SIZE, SIZE};
    SDL_Point center_{0, 0};
    const Character *target_ = nullptr; // 指向的target对象不能修改，但可以指向别的对象
};