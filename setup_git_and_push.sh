#!/bin/bash

# C++ Games 仓库初始化和推送脚本

set -e

echo "🚀 初始化 Git 仓库并推送到 GitHub..."

# 检查是否已经有 .git 目录
if [[ -d ".git" ]]; then
    echo "ℹ️  Git 仓库已存在，跳过初始化"
else
    echo "📝 初始化 Git 仓库..."
    git init
fi

# 配置用户信息（请根据实际情况修改）
echo "👤 配置 Git 用户信息..."
git config user.name "HelkeNarmo"
git config user.email "your-email@example.com"  # 请替换为你的邮箱

# 添加远程仓库
echo "🔗 添加远程仓库..."
if git remote get-url origin >/dev/null 2>&1; then
    echo "ℹ️  远程仓库已存在，更新 URL..."
    git remote set-url origin git@github.com:HelkeNarmo/cpp_games.git
else
    git remote add origin git@github.com:HelkeNarmo/cpp_games.git
fi

# 创建 .gitignore 文件（如果不存在）
if [[ ! -f ".gitignore" ]]; then
    echo "📄 创建 .gitignore 文件..."
    cat > .gitignore << 'EOF'
# 构建产物
*.o
main
main_debug
*.exe
*.dll
*.dylib

# macOS
.DS_Store
*.dSYM/
Contents/

# 临时文件
*.tmp
*.log

# IDE
.vscode/settings.json
.idea/

# 发布包
*.tar.gz
*.zip
release/
EOF
fi

# 添加所有文件
echo "📦 添加文件到 Git..."
git add .

# 提交代码
echo "💾 提交代码..."
git commit -m "Initial commit: C++ Game Workspace

🎮 C++游戏开发合集，包含：
- Type Tag: 多人打字竞速游戏
- Slime Survivor: 动作射击游戏
- Tic Tac Toe: 井字棋游戏

🚀 特性：
- 跨平台构建支持 (macOS/Windows/Linux)
- 智能 CI/CD (GitHub Actions)
- 自动发布包生成"

# 推送代码
echo "⬆️  推送代码到 GitHub..."
git branch -M main
git push -u origin main

echo ""
echo "🎉 成功！代码已推送到 GitHub"
echo "📋 查看仓库: https://github.com/HelkeNarmo/cpp_games"
echo "🤖 查看 Actions: https://github.com/HelkeNarmo/cpp_games/actions"
echo ""
echo "💡 后续推送代码:"
echo "   git add ."
echo "   git commit -m '更新说明'"
echo "   git push"