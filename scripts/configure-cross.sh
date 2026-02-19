#!/usr/bin/env bash
# configure-cross.sh — Generate Android cross-compilation .ini files from templates
#
# Reads cross/android-*.ini templates (containing @NDK@ and @NDK_HOST@ placeholders)
# and writes configured files to cross/local/ (gitignored).
#
# Usage:
#   ./scripts/configure-cross.sh                       # auto-detect everything
#   ANDROID_NDK_HOME=/path/to/ndk ./scripts/configure-cross.sh  # explicit NDK
#
# Environment variables:
#   ANDROID_NDK_HOME  — Path to Android NDK root (auto-detected if not set)
#   ANDROID_HOME      — Fallback: searches for NDK inside $ANDROID_HOME/ndk/

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CROSS_DIR="$ROOT_DIR/cross"
LOCAL_DIR="$CROSS_DIR/local"

# ---------- Detect host platform for NDK toolchain ----------

detect_ndk_host() {
    local os arch
    os="$(uname -s)"
    arch="$(uname -m)"

    case "$os" in
        Linux)  os="linux" ;;
        Darwin) os="darwin" ;;
        MINGW*|MSYS*|CYGWIN*) os="windows" ;;
        *) echo "Unsupported OS: $os" >&2; return 1 ;;
    esac

    case "$arch" in
        x86_64|amd64) arch="x86_64" ;;
        aarch64|arm64) arch="x86_64" ;;  # NDK ships x86_64 toolchain on ARM Macs (Rosetta)
        *) echo "Unsupported arch: $arch" >&2; return 1 ;;
    esac

    echo "${os}-${arch}"
}

# ---------- Detect Android NDK path ----------

detect_ndk_path() {
    # 1. Explicit environment variable
    if [ -n "${ANDROID_NDK_HOME:-}" ] && [ -d "$ANDROID_NDK_HOME" ]; then
        echo "$ANDROID_NDK_HOME"
        return
    fi

    # 2. Search inside ANDROID_HOME/ndk/ (pick latest version)
    if [ -n "${ANDROID_HOME:-}" ] && [ -d "$ANDROID_HOME/ndk" ]; then
        local latest
        latest=$(ls -1d "$ANDROID_HOME/ndk"/*/ 2>/dev/null | sort -V | tail -1)
        if [ -n "$latest" ]; then
            echo "${latest%/}"
            return
        fi
    fi

    # 3. Common default locations
    local search_dirs=(
        "$HOME/Library/Android/sdk/ndk"    # macOS default
        "$HOME/Android/Sdk/ndk"            # Linux default
        "/usr/local/lib/android/sdk/ndk"   # CI environments
    )
    for dir in "${search_dirs[@]}"; do
        if [ -d "$dir" ]; then
            local latest
            latest=$(ls -1d "$dir"/*/ 2>/dev/null | sort -V | tail -1)
            if [ -n "$latest" ]; then
                echo "${latest%/}"
                return
            fi
        fi
    done

    return 1
}

# ---------- Main ----------

NDK_HOST=$(detect_ndk_host)
echo "NDK host platform: $NDK_HOST"

if NDK_PATH=$(detect_ndk_path); then
    echo "NDK path: $NDK_PATH"
else
    echo "Error: Android NDK not found." >&2
    echo "Set ANDROID_NDK_HOME or install the NDK via Android Studio." >&2
    exit 1
fi

# Verify the toolchain exists
TOOLCHAIN_DIR="$NDK_PATH/toolchains/llvm/prebuilt/$NDK_HOST"
if [ ! -d "$TOOLCHAIN_DIR" ]; then
    echo "Error: Toolchain not found at $TOOLCHAIN_DIR" >&2
    echo "Check NDK installation and host platform." >&2
    exit 1
fi

# Generate configured cross-files in cross/local/
mkdir -p "$LOCAL_DIR"

count=0
for template in "$CROSS_DIR"/android-*.ini; do
    name=$(basename "$template")
    output="$LOCAL_DIR/$name"
    sed -e "s|@NDK@|$NDK_PATH|g" \
        -e "s|@NDK_HOST@|$NDK_HOST|g" \
        "$template" > "$output"
    echo "Generated: cross/local/$name"
    ((count++))
done

echo ""
echo "Done: $count cross-file(s) generated in cross/local/"
echo "Build with:"
echo "  meson setup builddir-android-arm64 --cross-file cross/local/android-arm64.ini"
