# C++ Game Workspace 🎮

一个C++游戏开发合集，包含多个小型游戏项目。使用SDL2等现代C++技术栈开发。

## 📁 项目列表

### 🎯 Type Tag - 多人打字竞速游戏
- **位置**: `type_tag/`
- **技术**: SDL2, C++17, 网络编程
- **特色**: 实时多人对战，角色在地图上竞速移动

### 🏹 Slime Survivor - 动作射击游戏
- **位置**: `slime_survivor/`
- **技术**: SDL2, C++17
- **特色**: 经典的生存射击玩法

### ⭕ Tic Tac Toe - 井字棋游戏
- **位置**: `tictactoe/`
- **技术**: C++17
- **特色**: 经典井字棋，支持双人游戏

## 🚀 快速开始

### 🎯 运行特定游戏

#### Type Tag (多人打字竞速)
```bash
# 方式1：一键运行
./run_game.sh type_tag

# 方式2：手动编译运行
cd type_tag
clang++ *.cpp -std=c++17 -g -o main $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_net)
./main
```

#### Slime Survivor (动作射击)
```bash
cd slime_survivor/src
clang++ *.cpp -std=c++17 -g -o main $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer)
./main
```

#### Tic Tac Toe (井字棋)
```bash
cd tictactoe
clang++ *.cpp -std=c++17 -g -o main
./main
```

### 🔧 跨平台构建

```bash
# 构建特定项目
./build_cross_platform.sh type_tag
./build_cross_platform.sh slime_survivor
./build_cross_platform.sh tictactoe

# 创建发布包
./create_release.sh type_tag
./create_release.sh slime_survivor
./create_release.sh tictactoe
```

## 📁 项目结构

```
cpp_game_workspace/
├── type_tag/                    # 🎯 多人打字竞速游戏
│   ├── main.cpp                 # 主程序入口
│   ├── *.cpp, *.h              # 游戏源码
│   ├── img/                     # 图片资源
│   ├── font/                    # 字体文件
│   └── libs/                    # 第三方库
├── slime_survivor/              # 🏹 动作射击游戏
│   ├── src/                     # 源代码
│   ├── img/                     # 图片资源
│   └── aud/                     # 音频资源
├── tictactoe/                   # ⭕ 井字棋游戏
│   └── *.cpp, *.h              # 游戏源码
├── build_cross_platform.sh      # 跨平台构建脚本
├── create_release.sh            # 发布包创建脚本
├── run_game.sh                  # 游戏运行脚本
├── .github/workflows/           # GitHub Actions 配置
│   └── build-games.yml         # 智能多项目构建
└── README.md                    # 项目文档
```

## 依赖要求

### macOS
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer, SDL2_net (通过Homebrew安装)
- C++17 编译器 (clang++)

### Windows (跨平台构建)
- MSYS2 + MinGW-w64
- SDL2 开发库 (通过pacman安装)

### Linux (跨平台构建)
- SDL2 开发库 (通过apt/yum安装)
- C++17 编译器 (g++)

## 🌐 跨平台支持

每个游戏项目都支持构建多个平台的发布版本：

### 支持的平台
- **macOS**: 原生应用 (Intel/Apple Silicon)
- **Windows**: exe可执行文件 (64位)
- **Linux**: 原生应用 (x86_64)

### 自动构建
配置了智能GitHub Actions工作流：
- **智能检测**: 检测哪些项目文件有变更，只构建有更新的项目
- **多平台并行**: 同时为macOS/Windows/Linux构建
- **增量构建**: 避免不必要的重复构建，提高效率
- **自动发布**: 推送到main分支时构建测试版本，创建标签时发布正式版本

## ⚠️ 注意事项

- **图形界面要求**：游戏需要在有图形界面的macOS环境中运行
- **SDL错误说明**：在终端环境中运行时会显示"SDL Init Error: The video driver did not add any displays"，这是正常现象
- **编译依赖**：需要安装SDL2相关库（通过Homebrew）
- **C++17支持**：需要支持C++17标准的编译器

## 🔧 故障排除

### 如果右键"Run Code"仍然失败：
1. 确保VS Code/Cursor已重启
2. 检查 `.vscode/tasks.json` 中的 "Run SDL2 Game" 任务是否设置为默认
3. 尝试手动运行 `./run_sdl2.sh`

### 如果编译失败：
1. 确保安装了所有依赖：`brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer`
2. 检查Homebrew路径：`/opt/homebrew/bin/pkg-config` 是否存在