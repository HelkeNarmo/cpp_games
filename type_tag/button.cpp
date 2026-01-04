#include <iostream>
#include "button.h"
#include "resource_cache.h"

const char* Button::BTN_BG = "./img/btn_bg.png";

bool Button::init(SDL_Renderer *r, shared_ptr<SDL_Texture> t, int x, int y, int tw, int th) {
    atlas_.texTxt = t;
    atlas_.texture = TextureCache::getInstance().get(r, BTN_BG);
    atlas_.atlasIndex = AtlasIndexCache::getInstance().get(atlas_.texture.get(), BTN_BG, FRAME_W, FRAME_H, 0, 0, 1, FRAME_NUM);
    dstRect_.x = x;
    dstRect_.y = y;
    dstRect_.w = FRAME_W * 2;
    dstRect_.h = FRAME_H * 2;
    txtRect_.x = x + (FRAME_W) / 2;
    txtRect_.y = y + (FRAME_H) / 2;
    txtRect_.w = tw;
    txtRect_.h = th;
    // printf("(%d, %d) - (%d, %d)\n", txtRect_.x, txtRect_.y, txtRect_.w, txtRect_.h);
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
                txtRect_.y = dstRect_.y + (FRAME_H + txtRect_.h / 2) / 2;
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (e.button.button == SDL_BUTTON_LEFT) {
            p = {e.button.x, e.button.y};
            if (SDL_PointInRect(&p, &dstRect_)) {
                // printf("SDL_PointInRect\n");
                if (isPressed_) {
                    if (onClick) onClick();
                    state_ = BtnState::Hover;
                } else {
                    state_ = BtnState::Idle;
                }
                txtRect_.y = dstRect_.y + (FRAME_H) / 2;
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

void Button::render(SDL_Renderer *r) {
    SDL_RenderCopy(r, atlas_.texture.get(), &atlas_.atlasIndex->frames[0][static_cast<int>(state_)], &dstRect_);
    SDL_RenderCopy(r, atlas_.texTxt.get(), nullptr, &txtRect_);
}