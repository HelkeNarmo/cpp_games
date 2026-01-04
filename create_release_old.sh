#!/bin/bash

# Type Tag 发布包创建脚本
# 自动创建包含所有必要文件的发布包

set -euo pipefail

echo "========================================="
echo "    Type Tag 发布包创建脚本"
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

PLATFORM=$(detect_platform)
echo "检测到平台: $PLATFORM"

# 检查构建产物是否存在
check_build_artifacts() {
    case "$PLATFORM" in
        "macos")
            if [[ ! -f "type_tag/type_tag_macos" ]]; then
                echo "❌ 未找到 macOS 可执行文件，请先运行构建脚本"
                echo "运行: ./build_cross_platform.sh"
                exit 1
            fi
            ;;
        "windows")
            if [[ ! -f "type_tag/type_tag.exe" ]]; then
                echo "❌ 未找到 Windows 可执行文件，请先运行构建脚本"
                echo "运行: ./build_cross_platform.sh"
                exit 1
            fi
            ;;
        "linux")
            if [[ ! -f "type_tag/type_tag_linux" ]]; then
                echo "❌ 未找到 Linux 可执行文件，请先运行构建脚本"
                echo "运行: ./build_cross_platform.sh"
                exit 1
            fi
            ;;
    esac
}

# 检查资源文件
check_resources() {
    local required_dirs=("img" "font")
    local required_files=("config.cfg")

    for dir in "${required_dirs[@]}"; do
        if [[ ! -d "type_tag/$dir" ]]; then
            echo "❌ 缺少必需目录: $dir"
            exit 1
        fi
    done

    for file in "${required_files[@]}"; do
        if [[ ! -f "type_tag/$file" ]]; then
            echo "❌ 缺少必需文件: $file"
            exit 1
        fi
    done
}

# 创建发布包
create_release_package() {
    local release_dir="release"
    local timestamp=$(date +"%Y%m%d_%H%M%S")

    echo "📦 创建发布目录..."
    rm -rf "$release_dir"
    mkdir -p "$release_dir"

    echo "📋 复制文件..."
    cp -r type_tag/img "$release_dir/"
    cp -r type_tag/font "$release_dir/"
    cp type_tag/config.cfg "$release_dir/"

    case "$PLATFORM" in
        "macos")
            cp type_tag/type_tag_macos "$release_dir/"
            echo "📦 创建 macOS 发布包..."
            cd "$release_dir"
            tar -czf "../type_tag_macos_${timestamp}.tar.gz" *
            cd ..
            echo "✅ macOS 发布包创建完成: type_tag_macos_${timestamp}.tar.gz"
            ;;

        "windows")
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
            zip -r "../type_tag_windows_${timestamp}.zip" *
            cd ..
            echo "✅ Windows 发布包创建完成: type_tag_windows_${timestamp}.zip"
            ;;

        "linux")
            cp type_tag/type_tag_linux "$release_dir/"
            echo "📦 创建 Linux 发布包..."
            cd "$release_dir"
            tar -czf "../type_tag_linux_${timestamp}.tar.gz" *
            cd ..
            echo "✅ Linux 发布包创建完成: type_tag_linux_${timestamp}.tar.gz"
            ;;
    esac

    # 清理临时目录
    rm -rf "$release_dir"

    echo ""
    echo "🎉 发布包创建完成！"
    echo "📂 发布包位置: $(pwd)"
    echo ""
    echo "📋 发布包内容:"
    case "$PLATFORM" in
        "macos")
            echo "  • type_tag_macos (可执行文件)"
            ;;
        "windows")
            echo "  • type_tag.exe (可执行文件)"
            ;;
        "linux")
            echo "  • type_tag_linux (可执行文件)"
            ;;
    esac
    echo "  • img/ (图片资源)"
    echo "  • font/ (字体文件)"
    echo "  • config.cfg (配置文件)"
    if [[ "$PLATFORM" == "windows" ]]; then
        echo "  • *.dll (Windows 动态链接库)"
    fi
}

# 主函数
main() {
    echo "🔍 检查构建产物..."
    check_build_artifacts

    echo "🔍 检查资源文件..."
    check_resources

    create_release_package
}

main "$@"