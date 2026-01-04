#pragma once
#include <SDL2/SDL.h>
#include "resource_cache.h"

struct ButtonAtlas {
    shared_ptr<SDL_Texture> texture;
    shared_ptr<const AtlasIndex> atlasIndex;
};

enum class ButtonType { START = 0, QUIT = 1 };

enum class BtnState { Idle = 0, Hover = 1, Pressed = 2 };

class Button {
public:
    static const char* ATLAS_PATH;
    static const int SIZE = 32;
    static const int FRAME_NUM = 3;

    function<void()> onClick;

    bool init(SDL_Renderer *r, ButtonType type, int x, int y);
    void handleMouseInput(const SDL_Event& e);
    void render();
protected:
    SDL_Renderer *renderer_{};
    SDL_Rect dstRect_;
    ButtonAtlas atlas_;
    BtnState state_ = BtnState::Idle;
    bool isPressed_ = false;
};