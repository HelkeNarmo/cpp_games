#include <iostream>
#include "button.h"
#include "resource_cache.h"

const char* Button::ATLAS_PATH = "../img/button.png";

bool Button::init(SDL_Renderer *r, ButtonType type, int x, int y) {
    renderer_ = r;
    atlas_.texture = TextureCache::getInstance().get(r, ATLAS_PATH);
    int startX = static_cast<int>(type) * SIZE * FRAME_NUM;
    atlas_.atlasIndex = AtlasIndexCache::getInstance().get(atlas_.texture.get(), ATLAS_PATH, SIZE, SIZE, startX, 0, 1, FRAME_NUM);
    dstRect_.x = x;
    dstRect_.y = y;
    dstRect_.w = SIZE;
    dstRect_.h = SIZE;
    return true;
}

void Button::handleMouseInput(const SDL_Event& e) {
    SDL_Point p;
    switch (e.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT) {
            p = {e.button.x, e.button.y};
            if (SDL_PointInRect(&p, &dstRect_)) {
                isPressed_ = true;
                state_ = BtnState::Pressed;
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (e.button.button == SDL_BUTTON_LEFT) {
            p = {e.button.x, e.button.y};
            if (SDL_PointInRect(&p, &dstRect_)) {
                printf("SDL_PointInRect\n");
                if (isPressed_) {
                    if (onClick) onClick();
                    state_ = BtnState::Hover;
                } else {
                    state_ = BtnState::Idle;
                }
            }
            isPressed_ = false;
        }
        break;
    case SDL_MOUSEMOTION:
        p = {e.motion.x, e.motion.y};
        if (!isPressed_) state_ = SDL_PointInRect(&p, &dstRect_) ? BtnState::Hover : BtnState::Idle;
        break;
    }
}

void Button::render() {
    SDL_RenderCopy(renderer_, atlas_.texture.get(), &atlas_.atlasIndex->frames[0][static_cast<int>(state_)], &dstRect_);
}