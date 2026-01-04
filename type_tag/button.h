#pragma once
#include <SDL2/SDL.h>
#include "resource_cache.h"

struct ButtonAtlas {
    shared_ptr<SDL_Texture> texture;
    shared_ptr<const AtlasIndex> atlasIndex;
    shared_ptr<SDL_Texture> texTxt;
};

enum class BtnState { Idle = 0, Hover = 1, Pressed = 2 };

class Button {
public:
    static const int FRAME_NUM = 3;

    function<void()> onClick;

    bool init(SDL_Renderer *r, shared_ptr<SDL_Texture> t, int x, int y, int tw, int th);
    void handleMouseInput(const SDL_Event& e);
    void render(SDL_Renderer *r);
private:
    static const char* BTN_BG;
    static const int FRAME_W = 50;
    static const int FRAME_H = 20;

    SDL_Rect dstRect_;
    SDL_Rect txtRect_;
    ButtonAtlas atlas_;
    BtnState state_ = BtnState::Idle;
    bool isPressed_ = false;
};