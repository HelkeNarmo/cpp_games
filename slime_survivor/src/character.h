#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>
#include <memory>
#include "constants.h"
#include "resource_cache.h"
using namespace std;

enum class AnimState { Idle, Walk, Attack, Hurt, Death };
enum class Dir {Down = 0, Up = 1, Left = 2, Right = 3};
struct AnimClip {
    shared_ptr<SDL_Texture> texture;
    shared_ptr<const AtlasIndex> atlasIndex;
    int fps{8};
    bool loop{true};
};

/**
 * 游戏中的角色基类
 */
class Character {
public:

    static const Uint32 DAMAGE_COOLDOWN = 500;

    virtual ~Character();

    static SDL_Point randomSpawnOutsidePos(int objW = 0, int objH = 0, int offset = 0);

    /**
     * 初始化
     */
    virtual bool init(SDL_Renderer *r, int initX, int initY);

    /**
     * 更新状态，每帧调用（传入时间，以使帧率无关行为）
     */
    virtual void update(int dt_ms);

    /**
     * 渲染，每帧调用
     */
    virtual void render();

    // 攻击
    virtual void attack();

    /**
     * 添加一个动画状态的 clip
     */
    bool addClip(AnimState state, const string &sheetPath, int frameW, int frameH, int rows, int cols, int fps, bool loop);

    // 基础控制
    void setPosition(double x, double y);
    void moveBy(double dx, double dy);
    void setDir(Dir d);
    void setSpeed(int s); // 每帧移动多少像素
    void setSize(int w, int h);
    void setStateFps(AnimState state, int fps);
    // 获得碰撞箱
    virtual const SDL_Rect* getHitRect() const { return &dstRect_; }

    // 切换状态（如果是同状态，就从第0帧开始）
    void setState(AnimState state);
    AnimState getState() const { return state_; }

    // 获取角色中心坐标
    SDL_Point center() const { return {int(x_ + dstRect_.w / 2), int(y_ + dstRect_.h / 2)}; }

    // 生命值
    int getHp() const { return hp; }
    void setHp(int h) { hp = h; }
    void damage(int d);
    bool isAllOver();

protected:
    SDL_Renderer *renderer_ = nullptr;
    unordered_map<AnimState, AnimClip> animClips_;

    AnimState state_ = AnimState::Idle;
    Dir dir_ = Dir::Down;
    float frameTimerMs_ = 0.0f;
    int speed_ = 150; // 像素/秒
    double x_ = 0.0, y_ = 0.0;
    int frameIdx_ = 0;
    int tickCnt_  = 0;
    SDL_Rect dstRect_{ 0, 0, 64, 64 };

    int hp = 100;
    Uint32 lastDamageTime = 0;

    void destroy();
    void nextState();

    virtual bool onInit(SDL_Renderer *r, int initX, int initY);
    virtual void onUpdate(int dt_ms);
    virtual void onRender();
    virtual void onAttack();
};

/**
 * 玩家角色
 */
class Hero final : public Character {
public:
    // 角色相关常量
    static const int WALK_NUM = 8;
    static const int IDLE_NUM = 6;
    static const int HURT_NUM = 5;
    static const int ATTACK_NUM = 10;
    static const int DEATH_NUM = 10;
    static const int SIZE = 64;
    static const int DEFAULT_FPS = 10;
    
    // 图片路径常量
    static const char* WALK_PATH;
    static const char* IDLE_PATH;
    static const char* HURT_PATH;
    static const char* ATTACK_PATH;
    static const char* DEATH_PATH;
    
    bool onInit(SDL_Renderer *r, int initX, int initY) override;
    void handleInput(const Uint8 *keys, bool attackTriggered, int dt_ms);
    void onUpdate(int dt_ms) override;
};

/**
 * 敌人角色
 */
class Enemy final : public Character {
public:
    // 角色相关常量
    static const int WALK_NUM = 8;
    static const int IDLE_NUM = 6;
    static const int HURT_NUM = 5;
    static const int DEATH_NUM = 10;
    static const int ATTACK_NUM = 10;
    static const int SIZE = 64;
    static const int DEFAULT_FPS = 10;
    static const int ATTACK_RANGE = 10;
    
    // 图片路径常量
    static const char* WALK_PATH;
    static const char* IDLE_PATH;
    static const char* HURT_PATH;
    static const char* DEATH_PATH;
    static const char* ATTACK_PATH;
    
    bool onInit(SDL_Renderer *r, int initX, int initY) override;
    void onUpdate(int dt_ms) override;
    void setTarget(Character *t) { target_ = t; }
    void checkTargetCollision();
protected:
    bool damageable() { return state_ == AnimState::Attack && frameIdx_ >= 7; }
private:
    Character *target_ = nullptr;
};
