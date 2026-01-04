#include "bullet.h"

const char* Guardian::ATLAS_PATH = "../img/bullets.png";
const SDL_Point Guardian::PIVOT = {Guardian::SIZE / 2, Guardian::SIZE / 2};

bool Guardian::init(SDL_Renderer *r) {
    renderer_ = r;
    bulletAtlas_.texture = TextureCache::getInstance().get(r, ATLAS_PATH);
    bulletAtlas_.atlasIndex = AtlasIndexCache::getInstance().get(bulletAtlas_.texture.get(), ATLAS_PATH, SIZE, SIZE, START_X, START_Y, 1, FRAME_NUM);
    return true;
}

void Guardian::update(int dt_ms) {
    if (target_) {
        center_ = target_->center();
    }
    orbitDeg_ += orbitSpeedDegPerSec_ * dt_ms / 1000.0f;
    if (orbitDeg_ >= 360.f) orbitDeg_ -= 360.f;
    dstRect_.x = center_.x + radius_ * cos(orbitDeg_ * M_PI / 180.0) - dstRect_.w / 2;
    dstRect_.y = center_.y + radius_ * sin(orbitDeg_ * M_PI / 180.0) - dstRect_.h / 2;
    spinDeg_ += spinSpeedDegPerSec_ * dt_ms / 1000.f;
    radius_ += radiusSpeedPerSec_ * dt_ms / 1000.0f;
    if (radius_ >= RADIUS_MAX) radiusSpeedPerSec_ = -abs(radiusSpeedPerSec_);
    if (radius_ <= RADIUS_MIN) radiusSpeedPerSec_ = abs(radiusSpeedPerSec_);
}

void Guardian::render() {
    auto srcRect_ = bulletAtlas_.atlasIndex->frames[0][frameIndex];
    SDL_RenderCopyEx(renderer_, bulletAtlas_.texture.get(), &srcRect_, &dstRect_, spinDeg_, &PIVOT, SDL_FLIP_NONE);
}

void Guardian::checkCollision(Enemy &enemy) {
    if (SDL_HasIntersection(&dstRect_, enemy.getHitRect())) {
        enemy.damage(10);
    }
}
