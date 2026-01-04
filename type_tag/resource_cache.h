#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

static inline void hash_combine(std::size_t& seed, std::size_t v) {
    seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

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
    void clear() { cache_.clear(); };

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
    void clear() { cache_.clear(); };

private:
    AtlasIndexCache() = default;

    unordered_map<string, weak_ptr<const AtlasIndex>> cache_;
};

struct TextKey {
    std::string s;
    SDL_Color c;
    int ptSize;
    TTF_Font* f;
    bool operator==(const TextKey& o) const {
        return ptSize == o.ptSize
            && f == o.f
            && c.r == o.c.r && c.g == o.c.g && c.b == o.c.b && c.a == o.c.a
            && s == o.s;
    }
};

struct TextKeyHash {
    std::size_t operator()(const TextKey& k) const {
        std::size_t h = 0;
        hash_combine(h, hash<std::string>{}(k.s));
        hash_combine(h, hash<int>{}(k.ptSize));
        hash_combine(h, hash<TTF_Font*>{}(k.f));
        std::uint32_t rgba =
            (std::uint32_t(k.c.r) << 24) |
            (std::uint32_t(k.c.g) << 16) |
            (std::uint32_t(k.c.b) <<  8) |
            (std::uint32_t(k.c.a)      );
        hash_combine(h, std::hash<std::uint32_t>{}(rgba));
        return h;
    }
};

class TextCache {
public:
    static TextCache &getInstance();

    ~TextCache() { clear(); }
    
    shared_ptr<SDL_Texture> get(SDL_Renderer* r, TTF_Font* f, const string &s, SDL_Color c, int ptSize, int *w = nullptr, int *h = nullptr);

    void clear() { cache_.clear(); }
private:
    TextCache() = default;

    struct Entry { shared_ptr<SDL_Texture> tex; int w, h; };
    unordered_map<TextKey, Entry, TextKeyHash> cache_;
};