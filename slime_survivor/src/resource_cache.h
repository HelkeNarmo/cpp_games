#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct AtlasIndex {
    vector<vector<SDL_Rect> > frames;
    int frameW{}, frameH{}, rows{}, cols{};
};

class TextureCache {
public:
    static TextureCache &getInstance();
    
    /**
     * 获取纹理，同一对renderer、path只加载一次
     */
    shared_ptr<SDL_Texture> get(SDL_Renderer *r, const string &path);

    /**
     * renderer销毁前调用
     */
    void clear();

private:
    TextureCache() = default;

    unordered_map<string, weak_ptr<SDL_Texture>> cache_;
};

class AtlasIndexCache {
public:
    static AtlasIndexCache &getInstance();
    
    /**
     * 获取动画集
     */
    shared_ptr<const AtlasIndex> get(SDL_Texture *atlas, const string &path, int frameW, int frameH, int startX, int startY, int rows, int cols);

    /**
     * renderer销毁前调用
     */
    void clear();

private:
    AtlasIndexCache() = default;

    unordered_map<string, weak_ptr<const AtlasIndex>> cache_;
};