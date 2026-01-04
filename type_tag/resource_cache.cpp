#include <sstream>
#include <stdexcept>
#include "resource_cache.h"

static auto make_tex_deleter() {
    return [](SDL_Texture* t) {
        if (t) {
            // SDL_Log("Destroy Texture: %p", t);
            SDL_DestroyTexture(t);
        }
    };
}

static SDL_Texture *loadTexture(SDL_Renderer *r, const string &path) {
    SDL_Surface *surf = IMG_Load(path.c_str());
    if (!surf) throw runtime_error("SDL Load Image Error: " + string(IMG_GetError()));
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!tex) throw runtime_error("SDL Create Texture Error: " + string(SDL_GetError()));
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

static AtlasIndex makeAtlasIndexFromAtlas(SDL_Texture *atlas, int frameW, int frameH, int startX, int startY, int rows, int cols) {
    int atlasW = 0, atlasH = 0;
    SDL_QueryTexture(atlas, nullptr, nullptr, &atlasW, &atlasH);
    AtlasIndex idx = {{}, frameW, frameH};
    auto &vec = idx.frames;
    vec.resize(rows);
    for (int row = 0; row < rows; ++row) {
        vec[row].reserve(cols);
        for (int c = 0; c < cols; ++c) {
            SDL_Rect rect = {startX + c * frameW, startY + row * frameH, frameW, frameH};
            vec[row].push_back(rect);
        }
    }
    idx.rows = rows;
    idx.cols = cols;
    return idx;
}

TextureCache &TextureCache::getInstance() {
    static TextureCache instance;
    return instance;
}

shared_ptr<SDL_Texture> TextureCache::get(SDL_Renderer *r, const string &path) {
    ostringstream oss;
    oss << r << '|' << path;
    const string key = oss.str();
    if (auto sp = cache_[key].lock()) return sp;
    auto raw = loadTexture(r, path);
    shared_ptr<SDL_Texture> tex(raw, make_tex_deleter());
    cache_[key] = tex;
    return tex;
}

AtlasIndexCache &AtlasIndexCache::getInstance() {
    static AtlasIndexCache instance;
    return instance;
}

shared_ptr<const AtlasIndex> AtlasIndexCache::get(SDL_Texture *atlas, const string &path, int frameW, int frameH, int startX, int startY, int rows, int cols) {
    ostringstream oss;
    oss << path << '#' << frameW << '#' << frameH << '#' << startX << '#' << startY << '#' << rows << '#' << cols;
    const string key = oss.str();
    if (auto sp = cache_[key].lock()) return sp;
    AtlasIndex raw = makeAtlasIndexFromAtlas(atlas, frameW, frameH, startX, startY, rows, cols);
    shared_ptr<const AtlasIndex> atlasIndex = make_shared<const AtlasIndex>(raw);
    cache_[key] = atlasIndex;
    return atlasIndex;
}

shared_ptr<SDL_Texture> TextCache::get(SDL_Renderer* r, TTF_Font* f, const string &s, SDL_Color c, int ptSize, int *w, int *h) {
    if (!r || !f) return nullptr;
    TextKey key{s, c, ptSize, f};
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        if (w) *w = it->second.w;
        if (h) *h = it->second.h;
        return it->second.tex;
    }
    TTF_SetFontSize(f, ptSize);
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, s.c_str(), c);
    if (!surf) return nullptr;
    SDL_Texture* raw = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!raw) return nullptr;
    shared_ptr<SDL_Texture> tex(raw, make_tex_deleter());
    Entry e{tex, 0, 0};
    SDL_QueryTexture(raw, nullptr, nullptr, &e.w, &e.h);
    if (w) *w = e.w;
    if (h) *h = e.h;
    cache_[key] = e;
    return tex;
}

TextCache &TextCache::getInstance() {
    static TextCache instance;
    return instance;
}