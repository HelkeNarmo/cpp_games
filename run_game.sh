#!/bin/bash

# 简单的Type Tag游戏运行脚本
# 直接在项目根目录运行：./run_game.sh

echo "🎮 Type Tag 游戏启动器"
echo "========================"

# 检查是否在正确的目录
if [[ ! -d "type_tag" ]]; then
    echo "❌ 错误：请在项目根目录运行此脚本"
    exit 1
fi

cd type_tag

echo "📦 编译中..."
/usr/bin/clang++ *.cpp -std=c++17 -g -o main $(/opt/homebrew/bin/pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_net)

if [[ $? -eq 0 ]]; then
    echo "✅ 编译成功！"
    echo "🚀 启动游戏..."
    echo ""
    ./main
else
    echo "❌ 编译失败！"
    exit 1
fi