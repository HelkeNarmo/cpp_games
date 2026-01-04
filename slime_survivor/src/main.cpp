#include<SDL2/SDL.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include<SDL2/SDL_image.h>
#include<string>
#include<vector>
#include "character.h"
#include "constants.h"
#include "bullet.h"
#include "audio_manager.h"
#include "button.h"
using namespace std;

const char* TITLE_PATH = "../img/title_bg.png";
const char* BG_PATH = "../img/map.png";

bool game_started = false;
bool running = true;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *bgTexture;
SDL_Texture *titleTexture;

Hero hero;
vector<Guardian> guardians;
vector<Enemy> enemies;

Button startBtn;
Button quitBtn;



void cleanup() {
    if (bgTexture) SDL_DestroyTexture(bgTexture);
    if (titleTexture) SDL_DestroyTexture(titleTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

/**
 * 初始化
 */
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL Init Error: %s", SDL_GetError());
        return false;
    }
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) == 0) {
        SDL_Log("SDL Image Init Error: %s", IMG_GetError());
        SDL_Quit();
        return false;
    }
    window = SDL_CreateWindow("Vampire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL Create Window Error: %s", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL Create Renderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    SDL_Surface *titleSurface = IMG_Load(TITLE_PATH);
    if (!titleSurface) {
        SDL_Log("SDL Load Image Error: %s", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_FreeSurface(titleSurface);
    if (!titleTexture) {
        SDL_Log("SDL Create Texture Error: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    SDL_Surface *bgSurface = IMG_Load(BG_PATH);
    if (!bgSurface) {
        SDL_Log("SDL Load Image Error: %s", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    if (!bgTexture) {
        SDL_Log("SDL Create Texture Error: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_BLEND);
    // 初始化音频
    if (!AudioManager::getInstance().init()) {
        return false;
    }
    // 初始化英雄角色
    if (!hero.init(renderer, SCREEN_WIDTH / 2 - Hero::SIZE / 2, SCREEN_HEIGHT / 2 - Hero::SIZE / 2)) {
        return false;
    }
    int enemyNum = 50;
    while (enemyNum--) {
        Enemy enemy;
        if (!enemy.init(renderer, SCREEN_WIDTH / 2 - Enemy::SIZE / 2, SCREEN_HEIGHT / 2 - Enemy::SIZE / 2)) {
            return false;
        }
        SDL_Point pos = Character::randomSpawnOutsidePos(Enemy::SIZE, Enemy::SIZE, 100);
        enemy.setPosition(pos.x, pos.y);
        enemy.setSpeed(60);
        enemy.setTarget(&hero);
        enemies.push_back(enemy);
    }
    int guardianNum = 3;
    while (guardianNum--) {
        Guardian guardian;
        if (!guardian.init(renderer)) {
            return false;
        }
        guardian.setOrbitDeg(guardianNum * 120.f);
        guardian.setTarget(&hero);
        guardians.push_back(guardian);
    }
    // 初始化开始按钮
    if (!startBtn.init(renderer, ButtonType::START, SCREEN_WIDTH / 2 - 200 / 2, SCREEN_HEIGHT / 2 - Button::SIZE / 2)) {
        return false;
    }
    startBtn.onClick = []() {
        AudioManager::getInstance().playBGM(2);
        game_started = true;
    };
    // 初始化退出按钮
    if (!quitBtn.init(renderer, ButtonType::QUIT, SCREEN_WIDTH / 2 + 200 / 2, SCREEN_HEIGHT / 2 - Button::SIZE / 2 )) {
        return false;
    }
    quitBtn.onClick = []() {
        running = false;
    };
    return true;
}

int main() {
    // 初始化游戏
    if (!init()) {
        SDL_Log("初始化失败！");
        return 1;
    }
    Uint32 nowTick;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 deltaTick;
    SDL_Event event;
    bool triggerAttack;
    while (running) {
        triggerAttack = false;
        if (SDL_WaitEventTimeout(&event, 5)) { // 阻塞一小会，或直到读取事件，节省CPU
            do {
                if (event.type == SDL_QUIT) running = false;
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) triggerAttack = true;
                startBtn.handleMouseInput(event);
                quitBtn.handleMouseInput(event);
            } while (SDL_PollEvent(&event));
        }

        if (!game_started) {
            // 渲染
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, titleTexture, NULL, NULL);
            startBtn.render();
            quitBtn.render();
        } else {
            // 更新数据
            nowTick = SDL_GetTicks();
            deltaTick = nowTick - lastTick;
            lastTick = nowTick;
            hero.handleInput(SDL_GetKeyboardState(NULL), triggerAttack, deltaTick);
            hero.update(deltaTick);
            for (auto &enemy : enemies) {
                enemy.update(deltaTick);
            }
            for (auto &guardian : guardians) {
                guardian.update(deltaTick);
            }

            // 渲染
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
            for (auto &enemy : enemies) {
                enemy.render();
            }
            hero.render();
            for (auto &guardian : guardians) {
                guardian.render();
            }
            
            // 碰撞检测
            for (auto &enemy : enemies) {
                enemy.checkTargetCollision();
            }
            for (auto &guardian : guardians) {
                for (int i = enemies.size() - 1; i >= 0; --i) {
                    Enemy &enemy = enemies[i];
                    guardian.checkCollision(enemy);
                    if (enemy.isAllOver()) {
                        swap(enemy, enemies.back());
                        enemies.pop_back();
                    }
                }
            }
            if (hero.isAllOver()) {
                running = false;
            }
        }
        SDL_RenderPresent(renderer);
    }
    cleanup();
    return 0;
}