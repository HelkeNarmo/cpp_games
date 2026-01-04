#include "./libs/httplib.h"
#include "path.h"
#include "player.h"
#include "resource_cache.h"
#include "dto.h"
#include "constants.h"
#include "button.h"
#include "network_server.h"
#include "network_client.h"
#include "network_udp.h"

#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <SDL2/SDL_ttf.h>

#ifdef _WIN32
#include <windows.h>
#endif

enum class Stage { MENU, SERVER_LIST, WAITING, READY, RACING };

static const string PATH_BG = "./img/map.png";
static const string PATH_NUM = "./img/numbers.png";
static const string PATH_GO = "./img/go.png";
static const string PATH_PLAYERS[4][2] = {
    {"./img/player1_idle.png", "./img/player1_walk.png"},
    {"./img/player2_idle.png", "./img/player2_walk.png"},
    {"./img/player3_idle.png", "./img/player3_walk.png"},
    {"./img/player4_idle.png", "./img/player4_walk.png"}
};
static const string PATH_FONT = "./font/CozetteVector.ttf";
static const Vector2 START_POS(320.0f, 320.0f);

bool running = true;
int countDown = 4;
Stage stage = Stage::MENU;

int playSize = 0;
PlayerDto me;
Player players[4]; // 下标id
vector<int> progresses; // 下标：id，值：progress
int totalProgress = 0;
Path path = Path({
    START_POS, {512,320}, {512,160}, {1088,160}, {1088,320}, {1248,320},
    {1248,672}, {832,672}, {832,608}, {320,608}, START_POS
});

int idxLine = 0;
int idxChar = 0;
string strText;
vector<string> strLineList;

SDL_Window *window;
SDL_Renderer *renderer;
shared_ptr<SDL_Texture> texBG;
int bgW = 0, bgH = 0;
SDL_Rect srcBG = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
SDL_Rect dstBG = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
shared_ptr<SDL_Texture> texNumbers;
shared_ptr<const AtlasIndex> atlasNumbers;
SDL_Rect *rectNumber;
shared_ptr<SDL_Texture> texGO;
Button btnServer;
Button btnClient;
Button btnStart;
vector<Button> serverButtons;
TTF_Font* font = nullptr;

string strAddrServ = "localhost";
int portServ = 25565;
int udpPort = 25566; // UDP广播端口

NetworkServer server;
NetworkClient client;
UDPBroadcaster broadcaster;
UDPListener listener;
vector<HostAnnounce> discoveredServers;
mutex serversMutex;

void showMessageBox(const string& title, const string& message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title.c_str(), message.c_str(), window);
}

void cleanup() {
    if (texBG) SDL_DestroyTexture(texBG.get());
    if (texNumbers) SDL_DestroyTexture(texNumbers.get());
    if (texGO) SDL_DestroyTexture(texGO.get());
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void onServerDiscovered(const HostAnnounce& announce);
void clearDiscoveredServers();
void connectToServer(const HostAnnounce& server);
void connectServer();

void loadResources(SDL_Renderer* renderer) {
    font = TTF_OpenFont(PATH_FONT.c_str(), 24);
    if (!font) {
        SDL_Log("SDL TTFOpenFont Error: %s", TTF_GetError());
        cleanup();
        exit(1);
    }
    texBG = TextureCache::getInstance().get(renderer, PATH_BG);
    SDL_QueryTexture(texBG.get(), nullptr, nullptr, &bgW, &bgH);
    shared_ptr<SDL_Texture> texTxt;
    int tw, th;
    texTxt = TextCache::getInstance().get(renderer, font, "Server", {255, 255, 255, 255}, 18, &tw, &th);
    btnServer.init(renderer, texTxt, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, tw, th);
    btnServer.onClick = ([&]() {
        // 启动HTTP服务器
        server.start("0.0.0.0", portServ, "Test text,Test text,Test text,Test text,Test text,Test text.", PLAYER_NUM_MAX);

        // 启动UDP广播
        broadcaster.start(udpPort, [&]() -> string {
            // 生成广播消息：TYPETAG|1|<httpPort>|<roomName>|<playerCnt>|<maxPlayer>
            return "TYPETAG|1|" + to_string(portServ) + "|Type Tag Room|" + to_string(progresses.size()) + "|" + to_string(PLAYER_NUM_MAX);
        });

        stage = Stage::WAITING;
    });
    texTxt = TextCache::getInstance().get(renderer, font, "Client", {255, 255, 255, 255}, 18, &tw, &th);
    btnClient.init(renderer, texTxt, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2, tw, th);
    btnClient.onClick = ([&]() {
        // 清除之前的服务器列表
        clearDiscoveredServers();

        // 启动UDP监听
        listener.start(udpPort, onServerDiscovered);

        // 切换到服务器列表界面
        stage = Stage::SERVER_LIST;
    });
    texTxt = TextCache::getInstance().get(renderer, font, "Start", {255, 255, 255, 255}, 18, &tw, &th);
    btnStart.init(renderer, texTxt, SCREEN_WIDTH / 2 - 51, SCREEN_HEIGHT / 2 - 17, tw, th);
    btnStart.onClick = ([&]() {
        if (progresses.size() == 0) return;
        stage = Stage::READY;
        countDown = 4;
    });
    texNumbers = TextureCache::getInstance().get(renderer, PATH_NUM);
    atlasNumbers = AtlasIndexCache::getInstance().get(texNumbers.get(), PATH_NUM, 19, 22, 0, 0, 1, 10);
    rectNumber = new SDL_Rect({SCREEN_WIDTH / 2 - 19, SCREEN_HEIGHT / 2 - 22, 19*2, 22*2});
    for (int i = 0; i < 4; i++) {
        players[i].init(renderer, PATH_PLAYERS[i][0], PATH_PLAYERS[i][1]);
        players[i].setPosition(START_POS);
    }
    // ifstream file("config.cfg");
    // if (!file.good()) {
    //     showMessageBox("Config Error", "config.cfg not found");
    //     cleanup();
    //     exit(1);
    // }
    // stringstream sstream;
    // sstream << file.rdbuf();
    // strAddrServ = sstream.str();
    // file.close();
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init Error: %s", SDL_GetError());
        return false;
    }
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) == 0) {
        SDL_Log("SDL Image Init Error: %s", IMG_GetError());
        SDL_Quit();
        return false;
    }
    window = SDL_CreateWindow("Type Tag", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("SDL CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    if (TTF_Init() == -1) {
        SDL_Log("SDL TTF Init Error: %s", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    loadResources(renderer);
    return true;
}

void onServerDiscovered(const HostAnnounce& announce) {
    lock_guard<mutex> lock(serversMutex);
    // 检查是否已存在该服务器
    for (auto& server : discoveredServers) {
        if (server.ip == announce.ip && server.httpPort == announce.httpPort) {
            server.playerCnt = announce.playerCnt;
            server.maxPlayer = announce.maxPlayer;
            return;
        }
    }
    // 添加新发现的服务器
    discoveredServers.push_back(announce);
}

void clearDiscoveredServers() {
    lock_guard<mutex> lock(serversMutex);
    discoveredServers.clear();
}

void connectToServer(const HostAnnounce& server) {
    strAddrServ = server.ip;
    portServ = server.httpPort;
    connectServer();
}

void connectServer() {
    if (!client.connectAndLogin(strAddrServ, portServ, me.id)) {
        showMessageBox("Login Error", "Login failed");
        exit(1);
    }
    if (!client.fetchText(strText)) {
        showMessageBox("Text Error", "Text fetch failed");
        exit(1);
    }
    stringstream sstream(strText);
    string strLine;
    while (getline(sstream, strLine)) {
        strLineList.push_back(strLine);
        totalProgress += (int)strLine.length();
    }
    client.startProgressLoop();
}

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;  // Avoid unused parameter warnings
    using namespace std::chrono;
    if (!init()) {
        return 1;
    }
    Timer countDownTimer;
    Camera cameraUI, cameraScene;
    cameraUI.setMaxSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    cameraScene.setMaxSize(bgW, bgH);
    cameraUI.setSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
    cameraScene.setSize(Vector2(SCREEN_WIDTH, SCREEN_HEIGHT));
    countDownTimer.setOneShot(false);
    countDownTimer.setWaitTime(1.0f);
    countDownTimer.setOnTimeout([&]() {
        countDown--;
        switch (countDown) {
        case 3: break;
        case 2: break;
        case 1: break;
        case 0:  break;
        case -1: stage = Stage::RACING; SDL_StartTextInput(); break;
        default: break;
        }
    });
    const nanoseconds frameDuration(1000000000 / 60);
    steady_clock::time_point lastTick = steady_clock::now();
    SDL_Event evt;
    string msgTitle = "";
    string msgBody = "";
    int txtW, txtH, txtTappedW, txtTappedH;
    shared_ptr<SDL_Texture> texTxt;
    shared_ptr<SDL_Texture> texTxtTapped;
    SDL_Rect dstTxt;
    SDL_Rect dstTxtTapped;
    string typed;
    while (running) {
        steady_clock::time_point frameStart = steady_clock::now();
        duration<float> delta = duration<float>(frameStart - lastTick);
        // 处理输入
        if (SDL_WaitEventTimeout(&evt, 5)) {
            do {
                if (evt.type == SDL_QUIT) running = false;
                if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (evt.type == SDL_TEXTINPUT) {
                    const string &strLine = strLineList[idxLine];
                    int len = strlen(evt.text.text);
                    if (strLine.substr(idxChar, len) == evt.text.text) {
                        idxChar += len;
                        me.progress += len;
                        client.submitProgress(me.progress);
                        if (idxChar >= strLine.size()) {
                            idxLine++;
                            idxChar = 0;
                        }
                    }
                }
                switch (stage) {
                case Stage::MENU:
                    btnServer.handleMouseInput(evt);
                    btnClient.handleMouseInput(evt);
                    break;
                case Stage::SERVER_LIST:
                    // 更新服务器按钮列表
                    {
                        lock_guard<mutex> lock(serversMutex);
                        // 如果服务器数量发生变化，重新创建按钮
                        if (serverButtons.size() != discoveredServers.size()) {
                            serverButtons.clear();
                            for (size_t i = 0; i < discoveredServers.size(); ++i) {
                                const auto& server = discoveredServers[i];
                                string buttonText = server.roomName + " (" + server.ip + ":" + to_string(server.httpPort) + ") " +
                                                  to_string(server.playerCnt) + "/" + to_string(server.maxPlayer);

                                shared_ptr<SDL_Texture> texTxt = TextCache::getInstance().get(renderer, font, buttonText, {255, 255, 255, 255}, 18, nullptr, nullptr);
                                Button btn;
                                int btnW, btnH;
                                TTF_SizeText(font, buttonText.c_str(), &btnW, &btnH);
                                btn.init(renderer, texTxt, SCREEN_WIDTH / 2 - btnW / 2, 200 + (int)i * 60, btnW, btnH);
                                btn.onClick = [server]() {
                                    connectToServer(server);
                                };
                                serverButtons.push_back(btn);
                            }
                        }
                        // 处理服务器按钮输入
                        for (auto& btn : serverButtons) {
                            btn.handleMouseInput(evt);
                        }
                    }
                    break;
                case Stage::WAITING:
                    btnStart.handleMouseInput(evt);
                    break;
                case Stage::READY:
                    break;
                case Stage::RACING:
                    break;
                }
                if (stage == Stage::WAITING) {
                    
                }
            } while (SDL_PollEvent(&evt));
        }
        // 处理游戏更新
        progresses = client.getLatestProgress();
        switch (stage) {
        case Stage::MENU:
            break;
        case Stage::WAITING:
            if (progresses.size() >= PLAYER_NUM_MAX) {
                stage = Stage::READY;
            }
            break;
        case Stage::READY:
            countDownTimer.onUpdate(delta.count());
            break;
        case Stage::RACING:
            if (!progresses.empty() && progresses[me.id] >= totalProgress) {
                running = false;
                msgTitle = "Game Over";
                msgBody = "You win!";
                SDL_StopTextInput();
            }
            for (int id = 0; id < PLAYER_NUM_MAX; id++) {
                players[id].setTarget(path.getPositionByProgress(progresses[id] * 1.0 / totalProgress));
                players[id].onUpdate(delta.count());
            }
            cameraScene.lookAt(players[me.id].getPosition());
            break;
        default:
            break;
        }
        // 处理渲染
        SDL_RenderClear(renderer);
        srcBG.x = cameraScene.getPosition().x;
        srcBG.y = cameraScene.getPosition().y;
        SDL_RenderCopy(renderer, texBG.get(), &srcBG, &dstBG);
        for (int i = 0; i < progresses.size(); ++i) {
            players[i].onRender(renderer, cameraScene);
        }
        switch (stage) {
        case Stage::MENU:
            btnServer.render(renderer);
            btnClient.render(renderer);
            break;
        case Stage::SERVER_LIST:
            // 显示标题
            texTxt = TextCache::getInstance().get(renderer, font, "Available Servers", {255, 255, 255, 255}, 32, &txtW, &txtH);
            if (texTxt) {
                dstTxt = { (SCREEN_WIDTH - txtW) / 2, 100, txtW, txtH };
                SDL_RenderCopy(renderer, texTxt.get(), nullptr, &dstTxt);
            }
            // 显示服务器按钮
            {
                lock_guard<mutex> lock(serversMutex);
                for (auto& btn : serverButtons) {
                    btn.render(renderer);
                }
                // 如果没有服务器，显示提示信息
                if (discoveredServers.empty()) {
                    texTxt = TextCache::getInstance().get(renderer, font, "Searching for servers...", {200, 200, 200, 255}, 24, &txtW, &txtH);
                    if (texTxt) {
                        dstTxt = { (SCREEN_WIDTH - txtW) / 2, 300, txtW, txtH };
                        SDL_RenderCopy(renderer, texTxt.get(), nullptr, &dstTxt);
                    }
                }
            }
            break;
        case Stage::WAITING:
            texTxt = TextCache::getInstance().get(renderer, font, "Waiting for other players...", {255, 255, 255, 255}, 32, &txtW, &txtH);
            if (texTxt) {
                dstTxt = { (SCREEN_WIDTH - txtW) / 2, SCREEN_HEIGHT - txtH, txtW, txtH };
                SDL_RenderCopy(renderer, texTxt.get(), nullptr, &dstTxt);
            }
            btnStart.render(renderer);
            break;
        case Stage::READY:
            if (0 <= countDown && countDown <= 3) {
                SDL_RenderCopy(renderer, texNumbers.get(), &atlasNumbers->frames[0][countDown], rectNumber);
            }
            break;
        case Stage::RACING:
            texTxt = TextCache::getInstance().get(renderer, font, strLineList[idxLine], {255, 255, 255, 255}, 32, &txtW, &txtH);
            if (texTxt) {
                dstTxt = { (SCREEN_WIDTH - txtW) / 2, SCREEN_HEIGHT - txtH, txtW, txtH };
                SDL_RenderCopy(renderer, texTxt.get(), nullptr, &dstTxt);
                if (idxChar > 0) {
                    texTxtTapped = TextCache::getInstance().get(renderer, font, strLineList[idxLine].substr(0, idxChar), {255, 130, 0, 255}, 32, &txtTappedW, &txtTappedH);
                    if (texTxtTapped) {
                        dstTxtTapped = { dstTxt.x, dstTxt.y, txtTappedW, txtTappedH };
                        SDL_RenderCopy(renderer, texTxtTapped.get(), nullptr, &dstTxtTapped);
                    }
                }
            }
            break;
        }
        SDL_RenderPresent(renderer);

        lastTick = frameStart;
        nanoseconds sleepDuration = frameDuration - (steady_clock::now() - frameStart);
        if (sleepDuration > nanoseconds(0)) this_thread::sleep_for(sleepDuration);
    }
    if (msgBody != "") {
        showMessageBox(msgTitle, msgBody);
    }
    client.stop();
    broadcaster.stop();
    listener.stop();
    cleanup();
    return 0;
}