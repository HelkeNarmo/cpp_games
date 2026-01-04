#include "character.h"
#include <string>
#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include "resource_cache.h"
#include "audio_manager.h"
using namespace std;

// Hero类静态常量初始化
const char* Hero::WALK_PATH = "../img/hero/hero_walk.png";
const char* Hero::IDLE_PATH = "../img/hero/hero_idle.png";
const char* Hero::HURT_PATH = "../img/hero/hero_hurt.png";
const char* Hero::ATTACK_PATH = "../img/hero/hero_attack.png";
const char* Hero::DEATH_PATH = "../img/hero/hero_death.png";

// Enemy类静态常量初始化
const char* Enemy::WALK_PATH = "../img/enemy/enemy_walk.png";
const char* Enemy::IDLE_PATH = "../img/enemy/enemy_idle.png";
const char* Enemy::HURT_PATH = "../img/enemy/enemy_hurt.png";
const char* Enemy::DEATH_PATH = "../img/enemy/enemy_death.png";
const char* Enemy::ATTACK_PATH = "../img/enemy/enemy_attack.png";

Character::~Character() {
    destroy();
}

SDL_Point Character::randomSpawnOutsidePos(int objW, int objH, int offset) {
    static mt19937 rng(random_device{}());
    uniform_int_distribution<int> pickSize(0, 3);
    const int side = pickSize(rng);
    uniform_int_distribution<int> distX(0, max(0, SCREEN_WIDTH - 1));
    uniform_int_distribution<int> distY(0, max(0, SCREEN_HEIGHT - 1));
    SDL_Point p{0, 0};
    switch (side) {
        case 0: { // 左侧
            p.x = -objW - offset;
            p.y = distY(rng);
            break;
        }
        case 1: { // 右侧
            p.x = SCREEN_WIDTH + offset;
            p.y = distY(rng);
            break;
        }
        case 2: { // 顶部
            p.x = distX(rng);
            p.y = -objH - offset;
            break;
        }
        case 3: { // 底部
            p.x = distX(rng);
            p.y = SCREEN_HEIGHT + offset;
            break;
        }
    }
    return p;
}

bool Character::init(SDL_Renderer *r, int initX, int initY) {
    renderer_ = r;
    setPosition(initX, initY);
    return onInit(r, initX, initY);
}

bool Character::onInit(SDL_Renderer *r, int initX, int initY) {
    return true;
}

bool Character::addClip(AnimState state, const string &sheetPath, int frameW, int frameH, int rows, int cols, int fps, bool loop) {
    try {
        AnimClip clip;
        clip.texture = TextureCache::getInstance().get(renderer_, sheetPath);
        if (!clip.texture) {
            SDL_Log("Failed to get texture for path: %s", sheetPath.c_str());
            return false;
        }
        clip.atlasIndex = AtlasIndexCache::getInstance().get(clip.texture.get(), sheetPath, frameW, frameH, 0, 0, rows, cols);
        if (!clip.atlasIndex) {
            SDL_Log("Failed to get anim set for path: %s", sheetPath.c_str());
            return false;
        }
        clip.fps = fps > 0 ? fps : 1;
        clip.loop = loop;
        animClips_[state] = move(clip);
        return true;
    } catch (const exception &e) {
        SDL_Log("Error loading clip for path %s: %s", sheetPath.c_str(), e.what());
        return false;
    }
}

void Character::update(int dt_ms) {
    auto it = animClips_.find(state_);
    if (it == animClips_.end()) return;
    const AnimClip &clip = it->second;
    frameTimerMs_ += dt_ms;
    const float frameDur = 1000.0f / clip.fps;
    while (frameTimerMs_ >= frameDur) {
        frameTimerMs_ -= frameDur;
        if (clip.loop) {
            frameIdx_ = (frameIdx_ + 1) % clip.atlasIndex.get()->cols;
        } else if (frameIdx_ + 1 < clip.atlasIndex.get()->cols) {
            frameIdx_++;
        } else {
            nextState();
        }
    }
    onUpdate(dt_ms);
}

void Character::onUpdate(int dt_ms) {}

void Character::render() {
    auto it = animClips_.find(state_);
    if (it == animClips_.end()) return;
    const AnimClip &clip = it->second;
    dstRect_.x = int(x_);
    dstRect_.y = int(y_);
    SDL_RenderCopy(renderer_, clip.texture.get(), &clip.atlasIndex->frames[static_cast<int>(dir_)][frameIdx_], &dstRect_);
    onRender();
}

void Character::onRender() {}

void Character::attack() {
    onAttack();
}

void Character::onAttack() {
    setState(AnimState::Attack);
}

void Character::nextState() {
    switch (state_) {
        case AnimState::Hurt:
        case AnimState::Attack:
            setState(AnimState::Idle);
            break;
        default:
            break;
    }
}

void Character::setPosition(double x, double y) {
    x_ = x;
    y_ = y;
}

void Character::moveBy(double dx, double dy) {
    x_ += dx;
    y_ += dy;
    if (x_ < 0) x_ = 0;
    if (y_ < 0) y_ = 0;
    if (x_ + dstRect_.w > SCREEN_WIDTH) x_ = SCREEN_WIDTH - dstRect_.w;
    if (y_ + dstRect_.h > SCREEN_HEIGHT) y_ = SCREEN_HEIGHT - dstRect_.h;
}

void Character::setDir(Dir d) {
    dir_ = d;
}

void Character::setSpeed(int s) {
    speed_ = s;
}

void Character::setSize(int w, int h) {
    dstRect_.w = w;
    dstRect_.h = h;
}

void Character::setState(AnimState state) {
    frameIdx_ = 0;
    if (state == state_) return;
    state_ = state;
    auto it = animClips_.find(state_);
    if (it == animClips_.end()) return;
    dstRect_.w = it->second.atlasIndex->frameW;
    dstRect_.h = it->second.atlasIndex->frameH;
}

void Character::setStateFps(AnimState state, int fps) {
    auto it = animClips_.find(state);
    if (it == animClips_.end()) return;
    it->second.fps = fps > 0 ? fps : 1;
}

void Character::damage(int d) {
    if (state_ == AnimState::Hurt || state_ == AnimState::Death) return;
    hp -= d;
    if (hp <= 0) {
        setState(AnimState::Death);
    } else {
        setState(AnimState::Hurt);
        AudioManager::getInstance().playHurt(0.5f);
    }
}

bool Character::isAllOver() {
    if (state_ != AnimState::Death) return false;
    auto it = animClips_.find(state_);
    if (it == animClips_.end()) return false;
    return frameIdx_ >= it->second.atlasIndex->cols - 1;
}

void Character::destroy() {
    for (auto &[st, clip] : animClips_) {
        clip.texture.reset();
        clip.atlasIndex.reset();
    }
    animClips_.clear();
}

bool Hero::onInit(SDL_Renderer *r, int initX, int initY) {
    setSize(SIZE, SIZE);
    // 添加所有动画状态
    bool success = true;
    success &= addClip(AnimState::Idle, IDLE_PATH, SIZE, SIZE, 4, IDLE_NUM, DEFAULT_FPS, true);
    success &= addClip(AnimState::Walk, WALK_PATH, SIZE, SIZE, 4, WALK_NUM, DEFAULT_FPS, true);
    success &= addClip(AnimState::Hurt, HURT_PATH, SIZE, SIZE, 4, HURT_NUM, DEFAULT_FPS, false);
    success &= addClip(AnimState::Attack, ATTACK_PATH, SIZE, SIZE, 4, ATTACK_NUM, DEFAULT_FPS, false);
    success &= addClip(AnimState::Death, DEATH_PATH, SIZE, SIZE, 4, DEATH_NUM, DEFAULT_FPS, false);
    if (!success) {
        SDL_Log("Hero初始化动画失败");
    }
    return success;
}

bool Enemy::onInit(SDL_Renderer *r, int initX, int initY) {
    setSize(SIZE, SIZE);
    // 添加所有动画状态
    bool success = true;
    success &= addClip(AnimState::Idle, IDLE_PATH, SIZE, SIZE, 4, IDLE_NUM, DEFAULT_FPS, true);
    success &= addClip(AnimState::Walk, WALK_PATH, SIZE, SIZE, 4, WALK_NUM, DEFAULT_FPS, true);
    success &= addClip(AnimState::Hurt, HURT_PATH, SIZE, SIZE, 4, HURT_NUM, DEFAULT_FPS * 2, false);
    success &= addClip(AnimState::Attack, ATTACK_PATH, SIZE, SIZE, 4, ATTACK_NUM, DEFAULT_FPS, false);
    success &= addClip(AnimState::Death, DEATH_PATH, SIZE, SIZE, 4, DEATH_NUM, DEFAULT_FPS, false);
    if (!success) {
        SDL_Log("Enemy初始化动画失败");
    }
    return success;
}

void Hero::handleInput(const Uint8 *keys, bool attackTriggered, int dt_ms) {
    if (state_ == AnimState::Attack) return;
    if (attackTriggered) {
        attack();
        return;
    }
    double dx = 0, dy = 0;
    if (keys[SDL_SCANCODE_W]) { setDir(Dir::Up); dy = -speed_ * dt_ms / 1000.0;}
    if (keys[SDL_SCANCODE_S]) { setDir(Dir::Down); dy = speed_ * dt_ms / 1000.0;}
    if (keys[SDL_SCANCODE_A]) { setDir(Dir::Left); dx = -speed_ * dt_ms / 1000.0;}
    if (keys[SDL_SCANCODE_D]) { setDir(Dir::Right); dx = speed_ * dt_ms / 1000.0;}
    double dist = sqrt(dx * dx + dy * dy);
    if (dist > 0) {
        dx /= dist;
        dy /= dist;
        moveBy(dx * speed_ * dt_ms / 1000.0, dy * speed_ * dt_ms / 1000.0); 
        if (state_ == AnimState::Idle) setState(AnimState::Walk);
    } else {
        if (state_ == AnimState::Walk) setState(AnimState::Idle);
    }
}

void Hero::onUpdate(int dt_ms) {}

void Enemy::onUpdate(int dt_ms) {
    if (!target_) return;
    if (state_ == AnimState::Attack) return;
    SDL_Point me = center();
    SDL_Point th = target_ -> center();
    double vx = th.x - me.x;
    double vy = th.y - me.y;
    double dist = sqrt(vx * vx + vy * vy);
    if (dist <= ATTACK_RANGE) {
        attack();
    } else {
        vx /= dist;
        vy /= dist;
        moveBy(vx * speed_ * dt_ms / 1000.0, vy * speed_ * dt_ms / 1000.0);
        if (state_ == AnimState::Idle) setState(AnimState::Walk);
    }
}

void Enemy::checkTargetCollision() {
    if (!target_ || !damageable()) return;
    if (SDL_HasIntersection(getHitRect(), target_->getHitRect())) {
        target_->damage(10);
    }
}