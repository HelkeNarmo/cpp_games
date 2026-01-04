#!/bin/bash

# C++ Game Workspace 跨平台构建脚本
# 支持构建多个游戏项目

set -euo pipefail

echo "========================================="
echo "    C++ Game Workspace 跨平台构建脚本"
echo "========================================="

# 检测平台
detect_platform() {
    case "$OSTYPE" in
        darwin*)
            echo "macos"
            ;;
        msys*|win32*)
            echo "windows"
            ;;
        linux*)
            echo "linux"
            ;;
        *)
            echo "unknown"
            ;;
    esac
}

# 检查项目是否存在
check_project() {
    local project="$1"
    if [[ ! -d "$project" ]]; then
        echo "❌ 错误：未找到项目目录 '$project'"
        echo "可用的项目："
        for dir in */; do
            if [[ -f "${dir}main.cpp" ]] || [[ -d "${dir}src" && -f "${dir}src/main.cpp" ]]; then
                echo "  - ${dir%/}"
            fi
        done
        exit 1
    fi
}

# Type Tag 项目构建
build_type_tag() {
    cd type_tag

    # 检查是否有源文件
    if ! ls *.cpp >/dev/null 2>&1; then
        echo "错误：在 type_tag 目录中未找到 .cpp 文件"
        exit 1
    fi

    case "$PLATFORM" in
        "macos")
            build_type_tag_macos
            ;;
        "windows")
            build_type_tag_windows
            ;;
        "linux")
            build_type_tag_linux
            ;;
        *)
            echo "❌ 不支持的平台: $OSTYPE"
            exit 1
            ;;
    esac
}

build_type_tag_macos() {
    local pkg_cmd="/opt/homebrew/bin/pkg-config"

    # 检查 pkg-config 是否存在
    if [[ ! -x "$pkg_cmd" ]]; then
        pkg_cmd="pkg-config"
        if ! command -v pkg-config >/dev/null 2>&1; then
            echo "错误：未找到 pkg-config，请确保已安装 SDL2 开发库"
            exit 1
        fi
    fi

    echo "🍎 编译 Type Tag (macOS)..."
    clang++ *.cpp -std=c++17 -O2 \
        $($pkg_cmd --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_net) \
        -mmacosx-version-min=10.12 \
        -o type_tag_macos

    echo "macOS 可执行文件已生成: type_tag/type_tag_macos"
}

build_type_tag_windows() {
    # 检查 MinGW 工具链
    if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
        echo "错误：未找到 MinGW-w64 工具链"
        echo "请安装 MinGW-w64: pacman -S mingw-w64-x86_64-gcc"
        exit 1
    fi

    echo "🪟 编译 Type Tag (Windows)..."
    x86_64-w64-mingw32-g++ *.cpp -std=c++17 -O2 \
        -static-libgcc -static-libstdc++ \
        $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_net 2>/dev/null || echo "-I/usr/x86_64-w64-mingw32/include -L/usr/x86_64-w64-mingw32/lib") \
        -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net \
        -mwindows \
        -o type_tag.exe

    echo "Windows 可执行文件已生成: type_tag/type_tag.exe"
}

build_type_tag_linux() {
    echo "🐧 编译 Type Tag (Linux)..."
    g++ *.cpp -std=c++17 -O2 \
        $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_net) \
        -o type_tag_linux

    echo "Linux 可执行文件已生成: type_tag/type_tag_linux"
}

# Slime Survivor 项目构建
build_slime_survivor() {
    cd slime_survivor/src

    # 检查是否有源文件
    if ! ls *.cpp >/dev/null 2>&1; then
        echo "错误：在 slime_survivor/src 目录中未找到 .cpp 文件"
        exit 1
    fi

    case "$PLATFORM" in
        "macos")
            build_slime_survivor_macos
            ;;
        "windows")
            build_slime_survivor_windows
            ;;
        "linux")
            build_slime_survivor_linux
            ;;
        *)
            echo "❌ 不支持的平台: $OSTYPE"
            exit 1
            ;;
    esac
}

build_slime_survivor_macos() {
    local pkg_cmd="/opt/homebrew/bin/pkg-config"

    if [[ ! -x "$pkg_cmd" ]]; then
        pkg_cmd="pkg-config"
        if ! command -v pkg-config >/dev/null 2>&1; then
            echo "错误：未找到 pkg-config，请确保已安装 SDL2 开发库"
            exit 1
        fi
    fi

    echo "🍎 编译 Slime Survivor (macOS)..."
    clang++ *.cpp -std=c++17 -O2 \
        $($pkg_cmd --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer) \
        -mmacosx-version-min=10.12 \
        -o slime_survivor_macos

    echo "macOS 可执行文件已生成: slime_survivor/src/slime_survivor_macos"
}

build_slime_survivor_windows() {
    # 检查 MinGW 工具链
    if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
        echo "错误：未找到 MinGW-w64 工具链"
        echo "请安装 MinGW-w64: pacman -S mingw-w64-x86_64-gcc"
        exit 1
    fi

    echo "🪟 编译 Slime Survivor (Windows)..."
    x86_64-w64-mingw32-g++ *.cpp -std=c++17 -O2 \
        -static-libgcc -static-libstdc++ \
        $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer 2>/dev/null || echo "-I/usr/x86_64-w64-mingw32/include -L/usr/x86_64-w64-mingw32/lib") \
        -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer \
        -mwindows \
        -o slime_survivor.exe

    echo "Windows 可执行文件已生成: slime_survivor/src/slime_survivor.exe"
}

build_slime_survivor_linux() {
    echo "🐧 编译 Slime Survivor (Linux)..."
    g++ *.cpp -std=c++17 -O2 \
        $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer) \
        -o slime_survivor_linux

    echo "Linux 可执行文件已生成: slime_survivor/src/slime_survivor_linux"
}

# Tic Tac Toe 项目构建
build_tictactoe() {
    cd tictactoe

    # 检查是否有源文件
    if ! ls *.cpp >/dev/null 2>&1; then
        echo "错误：在 tictactoe 目录中未找到 .cpp 文件"
        exit 1
    fi

    case "$PLATFORM" in
        "macos")
            build_tictactoe_macos
            ;;
        "windows")
            build_tictactoe_windows
            ;;
        "linux")
            build_tictactoe_linux
            ;;
        *)
            echo "❌ 不支持的平台: $OSTYPE"
            exit 1
            ;;
    esac
}

build_tictactoe_macos() {
    echo "🍎 编译 Tic Tac Toe (macOS)..."
    clang++ *.cpp -std=c++17 -O2 \
        -mmacosx-version-min=10.12 \
        -o tictactoe_macos

    echo "macOS 可执行文件已生成: tictactoe/tictactoe_macos"
}

build_tictactoe_windows() {
    # 检查 MinGW 工具链
    if ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
        echo "错误：未找到 MinGW-w64 工具链"
        echo "请安装 MinGW-w64: pacman -S mingw-w64-x86_64-gcc"
        exit 1
    fi

    echo "🪟 编译 Tic Tac Toe (Windows)..."
    x86_64-w64-mingw32-g++ *.cpp -std=c++17 -O2 \
        -static-libgcc -static-libstdc++ \
        -mwindows \
        -o tictactoe.exe

    echo "Windows 可执行文件已生成: tictactoe/tictactoe.exe"
}

build_tictactoe_linux() {
    echo "🐧 编译 Tic Tac Toe (Linux)..."
    g++ *.cpp -std=c++17 -O2 \
        -o tictactoe_linux

    echo "Linux 可执行文件已生成: tictactoe/tictactoe_linux"
}

# 主函数
main() {
    local project="$1"

    if [[ -z "$project" ]]; then
        echo "用法: $0 <项目名>"
        echo "例如: $0 type_tag"
        echo "      $0 slime_survivor"
        echo "      $0 tictactoe"
        echo ""
        echo "可用的项目："
        for dir in */; do
            if [[ -f "${dir}main.cpp" ]] || [[ -d "${dir}src" && -f "${dir}src/main.cpp" ]]; then
                echo "  - ${dir%/}"
            fi
        done
        exit 1
    fi

    PLATFORM=$(detect_platform)
    echo "检测到平台: $PLATFORM"

    check_project "$project"

    case "$project" in
        "type_tag")
            build_type_tag
            ;;
        "slime_survivor")
            build_slime_survivor
            ;;
        "tictactoe")
            build_tictactoe
            ;;
        *)
            echo "❌ 不支持的项目: $project"
            exit 1
            ;;
    esac

    echo "✅ $project 构建完成！"
}

# 调用主函数
main "$@"