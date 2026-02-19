#!/usr/bin/env bash
# build-all.sh — Build Giac for all supported platforms
#
# Usage:
#   ./scripts/build-all.sh                  # build everything
#   ./scripts/build-all.sh native android   # build specific platforms
#   ./scripts/build-all.sh --clean          # wipe build dirs first
#   ./scripts/build-all.sh --verbose        # show full compiler output
#
# Platform selectors: native, android, ios, windows, wasm
#
# Prerequisites (macOS):
#   brew install meson ninja gmp mpfr mingw-w64 emscripten
#   Android NDK path set in cross/android-*.ini files
#   Xcode with iOS/macOS SDKs for iOS/Catalyst builds

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

# ---------- Parse arguments ----------

CLEAN=false
VERBOSE=""
PLATFORMS=()

for arg in "$@"; do
    case "$arg" in
        --clean)  CLEAN=true ;;
        --verbose) VERBOSE="-v" ;;
        native|android|ios|windows|wasm)
            PLATFORMS+=("$arg") ;;
        *)
            echo "Unknown argument: $arg" >&2
            echo "Usage: $0 [--clean] [--verbose] [native] [android] [ios] [windows] [wasm]" >&2
            exit 1 ;;
    esac
done

# Default: build all platforms
if [ ${#PLATFORMS[@]} -eq 0 ]; then
    PLATFORMS=(native android ios windows wasm)
fi

# ---------- Helpers ----------

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m'

PASS=0
FAIL=0
SKIP=0
declare -a RESULTS=()

log_header() {
    echo ""
    echo -e "${BOLD}=== $1 ===${NC}"
}

record_result() {
    local target="$1"
    local status="$2"
    local details="${3:-}"
    RESULTS+=("$status|$target|$details")
    if [ "$status" = "PASS" ]; then
        ((PASS++))
        echo -e "  ${GREEN}PASS${NC} $target${details:+ — $details}"
    elif [ "$status" = "FAIL" ]; then
        ((FAIL++))
        echo -e "  ${RED}FAIL${NC} $target${details:+ — $details}"
    else
        ((SKIP++))
        echo -e "  ${YELLOW}SKIP${NC} $target${details:+ — $details}"
    fi
}

# Setup + compile a single target
# Arguments: builddir cross-file [extra-meson-args...]
build_target() {
    local builddir="$1"
    local crossfile="$2"
    shift 2
    local extra_args=("$@")

    if $CLEAN && [ -d "$builddir" ]; then
        rm -rf "$builddir"
    fi

    local setup_cmd=(meson setup "$builddir")
    if [ -n "$crossfile" ]; then
        setup_cmd+=(--cross-file "$crossfile")
    fi
    if [ ${#extra_args[@]} -gt 0 ]; then
        setup_cmd+=("${extra_args[@]}")
    fi

    # Setup (reconfigure if already exists, wipe on failure)
    if [ -d "$builddir" ]; then
        "${setup_cmd[@]}" --reconfigure >/dev/null 2>&1 || \
        "${setup_cmd[@]}" --wipe >/dev/null 2>&1
    else
        "${setup_cmd[@]}" >/dev/null 2>&1
    fi

    # Compile
    if [ -n "$VERBOSE" ]; then
        meson compile -C "$builddir" -v 2>&1
    else
        meson compile -C "$builddir" 2>&1
    fi
}

# Check if a command exists
require_cmd() {
    if ! command -v "$1" &>/dev/null; then
        return 1
    fi
    return 0
}

# List notable files in a build directory
list_artifacts() {
    local builddir="$1"
    local artifacts=()
    for f in "$builddir"/libgiac.a "$builddir"/libjavagiac.* \
             "$builddir"/libsimpleinterface.a "$builddir"/minigiac \
             "$builddir"/giacggb.wasm "$builddir"/giac.wasm.js; do
        if [ -f "$f" ]; then
            artifacts+=("$(basename "$f")")
        fi
    done
    echo "${artifacts[*]}"
}

# ---------- Check core prerequisites ----------

if ! require_cmd meson; then
    echo -e "${RED}Error: meson not found. Install with: brew install meson${NC}" >&2
    exit 1
fi

if ! require_cmd ninja; then
    echo -e "${RED}Error: ninja not found. Install with: brew install ninja${NC}" >&2
    exit 1
fi

echo -e "${BOLD}Giac Meson Build — All Platforms${NC}"
echo "Platforms: ${PLATFORMS[*]}"
echo "Root: $ROOT_DIR"

# ---------- Native desktop ----------

if [[ " ${PLATFORMS[*]} " == *" native "* ]]; then
    log_header "Native Desktop"
    if build_target builddir "" >/dev/null 2>&1; then
        artifacts=$(list_artifacts builddir)
        record_result "native-desktop" "PASS" "$artifacts"
        # Smoke test
        if [ -x builddir/minigiac ]; then
            result=$(echo '1+1' | ./builddir/minigiac 2>/dev/null | grep -o '^[0-9]*<<.*' | head -1 | sed 's/^[0-9]*<< *//' || true)
            if [ "$result" = "2" ]; then
                record_result "native-smoke-test" "PASS" "echo '1+1' -> 2"
            else
                record_result "native-smoke-test" "FAIL" "got: $result"
            fi
        fi
    else
        record_result "native-desktop" "FAIL"
    fi
fi

# ---------- Android (4 ABIs) ----------

if [[ " ${PLATFORMS[*]} " == *" android "* ]]; then
    log_header "Android"
    # Auto-configure cross-files if cross/local/ doesn't exist yet
    if [ ! -d cross/local ] || [ ! -f cross/local/android-arm64.ini ]; then
        echo "  Configuring Android cross-files..."
        if ! "$SCRIPT_DIR/configure-cross.sh"; then
            record_result "android (all)" "SKIP" "NDK not found — run: ANDROID_NDK_HOME=/path/to/ndk ./scripts/configure-cross.sh"
            PLATFORMS=("${PLATFORMS[@]/android/}")
        fi
    fi
    for abi in arm arm64 x86 x86_64; do
        crossfile="cross/local/android-${abi}.ini"
        builddir="builddir-android-${abi}"
        if [ ! -f "$crossfile" ]; then
            record_result "android-${abi}" "SKIP" "cross-file not found"
            continue
        fi
        if build_target "$builddir" "$crossfile" >/dev/null 2>&1; then
            artifacts=$(list_artifacts "$builddir")
            record_result "android-${abi}" "PASS" "$artifacts"
        else
            record_result "android-${abi}" "FAIL"
        fi
    done
fi

# ---------- iOS / Mac Catalyst (5 variants) ----------

if [[ " ${PLATFORMS[*]} " == *" ios "* ]]; then
    log_header "iOS / Mac Catalyst"
    for variant in ios-arm64 ios-sim-x86_64 ios-sim-arm64 \
                   maccatalyst-x86_64 maccatalyst-arm64; do
        crossfile="cross/${variant}.ini"
        builddir="builddir-${variant}"
        if [ ! -f "$crossfile" ]; then
            record_result "${variant}" "SKIP" "cross-file not found"
            continue
        fi
        if build_target "$builddir" "$crossfile" -Dwith_simpleinterface=true >/dev/null 2>&1; then
            artifacts=$(list_artifacts "$builddir")
            record_result "${variant}" "PASS" "$artifacts"
        else
            record_result "${variant}" "FAIL"
        fi
    done
fi

# ---------- Windows (MinGW cross) ----------

if [[ " ${PLATFORMS[*]} " == *" windows "* ]]; then
    log_header "Windows (MinGW cross)"
    if ! require_cmd x86_64-w64-mingw32-gcc; then
        record_result "windows-x86_64" "SKIP" "mingw-w64 not installed (brew install mingw-w64)"
    else
        if build_target builddir-windows cross/windows-clang64.ini >/dev/null 2>&1; then
            artifacts=$(list_artifacts builddir-windows)
            record_result "windows-x86_64" "PASS" "$artifacts"
        else
            record_result "windows-x86_64" "FAIL"
        fi
    fi
fi

# ---------- Emscripten / WASM ----------

if [[ " ${PLATFORMS[*]} " == *" wasm "* ]]; then
    log_header "Emscripten / WASM"
    if ! require_cmd emcc; then
        record_result "wasm32" "SKIP" "emscripten not installed (brew install emscripten)"
    else
        if build_target builddir-wasm cross/emscripten-wasm32.ini >/dev/null 2>&1; then
            artifacts=$(list_artifacts builddir-wasm)
            record_result "wasm32" "PASS" "$artifacts"
        else
            record_result "wasm32" "FAIL"
        fi
    fi
fi

# ---------- Summary ----------

echo ""
echo -e "${BOLD}========== Build Summary ==========${NC}"
printf "%-30s  %-6s  %s\n" "TARGET" "STATUS" "ARTIFACTS"
printf "%-30s  %-6s  %s\n" "------" "------" "---------"

for entry in "${RESULTS[@]}"; do
    IFS='|' read -r status target details <<< "$entry"
    case "$status" in
        PASS) color="$GREEN" ;;
        FAIL) color="$RED" ;;
        *)    color="$YELLOW" ;;
    esac
    printf "%-30s  ${color}%-6s${NC}  %s\n" "$target" "$status" "$details"
done

echo ""
echo -e "Total: ${GREEN}${PASS} passed${NC}, ${RED}${FAIL} failed${NC}, ${YELLOW}${SKIP} skipped${NC}"

# Exit with error if any build failed
if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
