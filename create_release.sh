#!/bin/bash

# C++ Game Workspace 发布包创建脚本
# 支持为不同项目创建发布包

set -euo pipefail

echo "========================================="
echo "    C++ Game Workspace 发布包创建脚本"
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

# 检查构建产物是否存在
check_build_artifacts() {
    local project="$1"

    case "$project-$PLATFORM" in
        "type_tag-macos")
            if [[ ! -f "type_tag/type_tag_macos" ]]; then
                echo "❌ 未找到 type_tag macOS 可执行文件，请先运行: ./build_cross_platform.sh type_tag"
                exit 1
            fi
            ;;
        "type_tag-windows")
            if [[ ! -f "type_tag/type_tag.exe" ]]; then
                echo "❌ 未找到 type_tag Windows 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
        "type_tag-linux")
            if [[ ! -f "type_tag/type_tag_linux" ]]; then
                echo "❌ 未找到 type_tag Linux 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
        "slime_survivor-macos")
            if [[ ! -f "slime_survivor/src/slime_survivor_macos" ]]; then
                echo "❌ 未找到 slime_survivor macOS 可执行文件，请先运行: ./build_cross_platform.sh slime_survivor"
                exit 1
            fi
            ;;
        "slime_survivor-windows")
            if [[ ! -f "slime_survivor/src/slime_survivor.exe" ]]; then
                echo "❌ 未找到 slime_survivor Windows 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
        "slime_survivor-linux")
            if [[ ! -f "slime_survivor/src/slime_survivor_linux" ]]; then
                echo "❌ 未找到 slime_survivor Linux 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
        "tictactoe-macos")
            if [[ ! -f "tictactoe/tictactoe_macos" ]]; then
                echo "❌ 未找到 tictactoe macOS 可执行文件，请先运行: ./build_cross_platform.sh tictactoe"
                exit 1
            fi
            ;;
        "tictactoe-windows")
            if [[ ! -f "tictactoe/tictactoe.exe" ]]; then
                echo "❌ 未找到 tictactoe Windows 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
        "tictactoe-linux")
            if [[ ! -f "tictactoe/tictactoe_linux" ]]; then
                echo "❌ 未找到 tictactoe Linux 可执行文件，请先运行构建脚本"
                exit 1
            fi
            ;;
    esac
}

# 检查资源文件
check_resources() {
    local project="$1"
    echo "检查 $project 的资源文件..."
    # 暂时跳过资源检查，后面再完善
}

# 创建发布包
create_release_package() {
    local project="$1"
    local release_dir="release_${project}"
    local timestamp=$(date +"%Y%m%d_%H%M%S")

    echo "📦 创建发布目录..."
    rm -rf "$release_dir"
    mkdir -p "$release_dir"

    echo "📋 复制文件..."

    case "$project" in
        "type_tag")
            cp -r type_tag/img "$release_dir/"
            cp -r type_tag/font "$release_dir/"
            cp type_tag/config.cfg "$release_dir/"
            ;;
        "slime_survivor")
            cp -r slime_survivor/img "$release_dir/"
            cp -r slime_survivor/aud "$release_dir/"
            ;;
        "tictactoe")
            # tictactoe 不需要额外资源文件
            ;;
    esac

    case "$project-$PLATFORM" in
        "type_tag-macos")
            cp type_tag/type_tag_macos "$release_dir/"
            echo "📦 创建 macOS 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_macos_${timestamp}.tar.gz" *
            cd ..
            echo "✅ macOS 发布包创建完成: ${project}_macos_${timestamp}.tar.gz"
            ;;

        "type_tag-windows")
            cp type_tag/type_tag.exe "$release_dir/"

            # 复制 Windows 所需的 DLL 文件（如果存在）
            echo "📋 复制 Windows DLL 文件..."
            if [[ -d "/usr/x86_64-w64-mingw32/bin" ]]; then
                # 常见的 SDL2 DLL 文件
                local dll_files=(
                    "SDL2.dll"
                    "SDL2_image.dll"
                    "SDL2_ttf.dll"
                    "SDL2_mixer.dll"
                    "SDL2_net.dll"
                    "libgcc_s_seh-1.dll"
                    "libstdc++-6.dll"
                    "libwinpthread-1.dll"
                )
                for dll in "${dll_files[@]}"; do
                    if [[ -f "/usr/x86_64-w64-mingw32/bin/$dll" ]]; then
                        cp "/usr/x86_64-w64-mingw32/bin/$dll" "$release_dir/" 2>/dev/null || true
                    fi
                done
            fi

            echo "📦 创建 Windows 发布包..."
            cd "$release_dir"
            zip -r "../${project}_windows_${timestamp}.zip" *
            cd ..
            echo "✅ Windows 发布包创建完成: ${project}_windows_${timestamp}.zip"
            ;;

        "type_tag-linux")
            cp type_tag/type_tag_linux "$release_dir/"
            echo "📦 创建 Linux 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_linux_${timestamp}.tar.gz" *
            cd ..
            echo "✅ Linux 发布包创建完成: ${project}_linux_${timestamp}.tar.gz"
            ;;

        "slime_survivor-macos")
            cp slime_survivor/src/slime_survivor_macos "$release_dir/"
            echo "📦 创建 macOS 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_macos_${timestamp}.tar.gz" *
            cd ..
            echo "✅ macOS 发布包创建完成: ${project}_macos_${timestamp}.tar.gz"
            ;;

        "slime_survivor-windows")
            cp slime_survivor/src/slime_survivor.exe "$release_dir/"

            # 复制 Windows 所需的 DLL 文件
            echo "📋 复制 Windows DLL 文件..."
            if [[ -d "/usr/x86_64-w64-mingw32/bin" ]]; then
                local dll_files=(
                    "SDL2.dll"
                    "SDL2_image.dll"
                    "SDL2_ttf.dll"
                    "SDL2_mixer.dll"
                    "libgcc_s_seh-1.dll"
                    "libstdc++-6.dll"
                    "libwinpthread-1.dll"
                )
                for dll in "${dll_files[@]}"; do
                    if [[ -f "/usr/x86_64-w64-mingw32/bin/$dll" ]]; then
                        cp "/usr/x86_64-w64-mingw32/bin/$dll" "$release_dir/" 2>/dev/null || true
                    fi
                done
            fi

            echo "📦 创建 Windows 发布包..."
            cd "$release_dir"
            zip -r "../${project}_windows_${timestamp}.zip" *
            cd ..
            echo "✅ Windows 发布包创建完成: ${project}_windows_${timestamp}.zip"
            ;;

        "slime_survivor-linux")
            cp slime_survivor/src/slime_survivor_linux "$release_dir/"
            echo "📦 创建 Linux 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_linux_${timestamp}.tar.gz" *
            cd ..
            echo "✅ Linux 发布包创建完成: ${project}_linux_${timestamp}.tar.gz"
            ;;

        "tictactoe-macos")
            cp tictactoe/tictactoe_macos "$release_dir/"
            echo "📦 创建 macOS 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_macos_${timestamp}.tar.gz" *
            cd ..
            echo "✅ macOS 发布包创建完成: ${project}_macos_${timestamp}.tar.gz"
            ;;

        "tictactoe-windows")
            cp tictactoe/tictactoe.exe "$release_dir/"

            # 复制 Windows 所需的 DLL 文件
            echo "📋 复制 Windows DLL 文件..."
            if [[ -d "/usr/x86_64-w64-mingw32/bin" ]]; then
                local dll_files=(
                    "libgcc_s_seh-1.dll"
                    "libstdc++-6.dll"
                    "libwinpthread-1.dll"
                )
                for dll in "${dll_files[@]}"; do
                    if [[ -f "/usr/x86_64-w64-mingw32/bin/$dll" ]]; then
                        cp "/usr/x86_64-w64-mingw32/bin/$dll" "$release_dir/" 2>/dev/null || true
                    fi
                done
            fi

            echo "📦 创建 Windows 发布包..."
            cd "$release_dir"
            zip -r "../${project}_windows_${timestamp}.zip" *
            cd ..
            echo "✅ Windows 发布包创建完成: ${project}_windows_${timestamp}.zip"
            ;;

        "tictactoe-linux")
            cp tictactoe/tictactoe_linux "$release_dir/"
            echo "📦 创建 Linux 发布包..."
            cd "$release_dir"
            tar -czf "../${project}_linux_${timestamp}.tar.gz" *
            cd ..
            echo "✅ Linux 发布包创建完成: ${project}_linux_${timestamp}.tar.gz"
            ;;
    esac

    # 清理临时目录
    rm -rf "$release_dir"

    echo ""
    echo "🎉 $project 发布包创建完成！"
    echo "📂 发布包位置: $(pwd)"
    echo ""
    echo "📋 发布包内容:"
    case "$PLATFORM" in
        "macos")
            echo "  • ${project}_macos (可执行文件)"
            ;;
        "windows")
            echo "  • ${project}.exe (可执行文件)"
            ;;
        "linux")
            echo "  • ${project}_linux (可执行文件)"
            ;;
    esac

    case "$project" in
        "type_tag")
            echo "  • img/ (图片资源)"
            echo "  • font/ (字体文件)"
            echo "  • config.cfg (配置文件)"
            ;;
        "slime_survivor")
            echo "  • img/ (图片资源)"
            echo "  • aud/ (音频资源)"
            ;;
        "tictactoe")
            echo "  • (无额外资源文件)"
            ;;
    esac

    if [[ "$PLATFORM" == "windows" ]]; then
        echo "  • *.dll (Windows 动态链接库)"
    fi
}

# 主函数
main() {
    local project="${1:-}"

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
    check_build_artifacts "$project"
    check_resources "$project"
    create_release_package "$project"
}

main "$@"