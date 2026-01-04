#!/usr/bin/env bash
set -euo pipefail

# Type Tag 游戏编译运行脚本
# 自动检测项目并编译所有源文件

echo "[Type Tag] 开始编译..."

# 获取脚本所在目录
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

# 检查type_tag目录
if [[ ! -d "$ROOT_DIR/type_tag" ]]; then
  echo "[Type Tag] 错误：未找到 type_tag 目录"
  exit 2
fi

cd "$ROOT_DIR/type_tag"

# 检查是否有源文件
if ! ls *.cpp >/dev/null 2>&1; then
  echo "[Type Tag] 错误：在 type_tag 目录中未找到 .cpp 文件"
  exit 2
fi

# 当前系统主次版本号（用于设置部署目标，避免 minOS 警告）
OSV=$(sw_vers -productVersion | awk -F. '{print $1"."$2}')
PKG="/opt/homebrew/bin/pkg-config"

# 编译所有 .cpp 文件
echo "[Type Tag] 编译所有源文件..."
/usr/bin/clang++ *.cpp -std=c++17 -g -o main \
  $($PKG --cflags --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf SDL2_net) \
  -mmacosx-version-min="$OSV"

if [[ $? -eq 0 ]]; then
  echo "[Type Tag] ✅ 编译成功！"
  echo "[Type Tag] 🚀 启动游戏..."
  exec ./main
else
  echo "[Type Tag] ❌ 编译失败！"
  exit 1
fi
