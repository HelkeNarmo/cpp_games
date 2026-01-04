#pragma once

#include "timer.h"
#include "vector2.h"
#include "resource_cache.h"
#include "camera.h"

#include <vector>
#include <functional>

class Animation {
public:
    Animation() {
        timer.setOneShot(false);
        timer.setOnTimeout([&]() {
            idxFrame++;
            if (idxFrame >= atlasIndex->frames.size()) {
                idxFrame = isLoop? 0 : atlasIndex->frames.size() - 1;
                if (!isLoop && onFinished) {
                    onFinished();
                }
            }
        });
    }
    void reset() {
        timer.restart();
        idxFrame = 0;
    }
    void setPosition(const Vector2& pos) {
        position = pos;
    }
    void setLoop(bool loop) {
        isLoop = loop;
    }
    void setInterval(float interval) {
        timer.setWaitTime(interval);
    }
    void setRowFrame(size_t row) {
        rowFrame = row;
    }
    void setOnFinished(std::function<void()> onFinished) {
        this->onFinished = onFinished;
    }
    void setAtlas(SDL_Renderer *renderer, const string &path, int frameW, int frameH, int startX, int startY, int rows, int cols) {
        texture = TextureCache::getInstance().get(renderer, path);
        atlasIndex = AtlasIndexCache::getInstance().get(texture.get(), path, frameW, frameH, startX, startY, rows, cols);
        dstRect.w = frameW;
        dstRect.h = frameH;
    }
    void onUpdate(float deltaTime) {
        timer.onUpdate(deltaTime);
    }
    void onRender(SDL_Renderer *renderer, const Camera& camera) {
        dstRect.x = position.x - camera.getPosition().x;
        dstRect.y = position.y - camera.getPosition().y;
        SDL_RenderCopy(renderer, texture.get(), &atlasIndex->frames[rowFrame][idxFrame], &dstRect);
    }
private:
    Timer timer;
    Vector2 position;
    bool isLoop = true;
    size_t rowFrame = 0;
    size_t idxFrame = 0;
    shared_ptr<SDL_Texture> texture;
    shared_ptr<const AtlasIndex> atlasIndex;
    std::function<void()> onFinished;
    SDL_Rect dstRect{};
};