#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#include <string>
using namespace std;

const int WIDTH = 480;
const int HEIGHT = 480;

const Uint8 LINE_WIDTH = 6;
const Uint32 COLOR_WHITE = 0xFFFFFFFF;
const Uint32 COLOR_BG = 0xFF1C1C1C;

char board[3][3] = {
    {'-', '-', '-'},
    {'-', '-', '-'},
    {'-', '-', '-'}
};

char moves[2] = {'X', 'O'};
int curMove = 1;

/**
 * @brief Check if there is a win in the game.
 * @return true if there is a win, false otherwise.
 */
bool checkWin() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != '-') {
            return true;
        }
    }
    for (int j = 0; j < 3; ++j) {
        if (board[0][j] == board[1][j] && board[1][j] == board[2][j] && board[0][j] != '-') {
            return true;
        }
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != '-') {
        return true;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != '-') {
        return true;
    }
    return false;
}

/**
 * @brief Check if the game is a draw.
 * @return true if the game is a draw, false otherwise.
 */
bool checkDraw() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == '-') {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Show a game over message box.
 * @param window The SDL window to display the message box on.
 * @param message The message to display in the message box.
 * @return int The ID of the button clicked by the user.
 */
int showGameOver(SDL_Window *window, string message) {
    SDL_MessageBoxButtonData btns[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "确定"}
    };
    SDL_MessageBoxData data = {};
    data.flags = SDL_MESSAGEBOX_INFORMATION;
    data.window = window;
    data.title = "游戏结束";
    data.message = message.c_str();
    data.numbuttons = SDL_arraysize(btns);
    data.buttons = btns;
    int btnId = 0;
    if (SDL_ShowMessageBox(&data, &btnId) != 0) {
        SDL_Log("Show GameOver Error: %s", SDL_GetError());
        return -1;
    }
    return btnId;
}

// 绘制网格线
void drawGrid(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 1; i < 3; ++i) {
        thickLineColor(renderer, i * WIDTH / 3, 0, i * WIDTH / 3, HEIGHT, LINE_WIDTH, COLOR_WHITE);
        thickLineColor(renderer, 0, i * HEIGHT / 3, WIDTH, i * HEIGHT / 3, LINE_WIDTH, COLOR_WHITE);
    }
}

// 绘制棋子
void drawPieces(SDL_Renderer *renderer) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == 'X') {
                thickLineColor(renderer, j * WIDTH / 3, i * HEIGHT / 3, (j + 1) * WIDTH / 3, (i + 1) * HEIGHT / 3, LINE_WIDTH, COLOR_WHITE);
                thickLineColor(renderer, (j + 1) * WIDTH / 3, i * HEIGHT / 3, j * WIDTH / 3, (i + 1) * HEIGHT / 3, LINE_WIDTH, COLOR_WHITE);
            } else if (board[i][j] == 'O') {
                filledCircleColor(renderer, j * WIDTH / 3 + WIDTH / 6, i * HEIGHT / 3 + HEIGHT / 6, WIDTH / 6, COLOR_WHITE);
                filledCircleColor(renderer, j * WIDTH / 3 + WIDTH / 6, i * HEIGHT / 3 + HEIGHT / 6, WIDTH / 6 - LINE_WIDTH, COLOR_BG);
            }
        }
    }
}

void onMouseClick(int x, int y) {
    int row = y / (HEIGHT / 3);
    int col = x / (WIDTH / 3);
    if (row < 0 || row >= 3 || col < 0 || col >= 3) return;
    if (board[row][col] != '-') return;
    curMove = (curMove + 1) % 2;
    board[row][col] = moves[curMove];
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL Init Error: %s", SDL_GetError());
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("Narmo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL Create Window Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL Create Renderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    bool running = true;
    string message = "";
    SDL_Event event;
    Uint32 lastTick;
    Uint32 deltaTick;
    while (running) {
        lastTick = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                onMouseClick(event.button.x, event.button.y);
            }
        }
        SDL_SetRenderDrawColor(renderer, (COLOR_BG >> 16) & 0xFF, (COLOR_BG >> 8) & 0xFF, (COLOR_BG >> 0) & 0xFF, (COLOR_BG >> 24) & 0xFF);
        SDL_RenderClear(renderer);

        if (checkWin()) {
            message = string("恭喜玩家 ") + moves[curMove] + " 获胜！";
            running = false;
        } else if (checkDraw()) {
            message = "平局！";
            running = false;
        }

        drawGrid(renderer);
        drawPieces(renderer);
        SDL_RenderPresent(renderer);
        deltaTick = SDL_GetTicks() - lastTick;
        if (deltaTick < 1000 / 60) {
            SDL_Delay(1000 / 60 - deltaTick);
        }
    }
    if (message != "") {
        showGameOver(window, message);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
