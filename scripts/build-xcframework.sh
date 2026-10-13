#!/usr/bin/env bash
# build-xcframework.sh — Assemble Giac.xcframework from Meson build outputs
#
# Usage: ./scripts/build-xcframework.sh [output-dir]
#
# Expects Meson build directories:
#   builddir-ios-arm64/
#   builddir-ios-sim-x86_64/
#   builddir-ios-sim-arm64/
#   builddir-maccatalyst-x86_64/
#   builddir-maccatalyst-arm64/
#
# Each must contain libgiac.a and libsimpleinterface.a

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="${1:-${ROOT_DIR}/build/framework}"

PREBUILT="${ROOT_DIR}/src/jni/prebuilt"
HEADERS="${ROOT_DIR}/src/simpleInterface/headers"
FRAMEWORK_SRC="${ROOT_DIR}/src/simpleInterface/framework"

MERGED="${OUTPUT_DIR}/merged"
FRAMEWORKS="${OUTPUT_DIR}/frameworks"
XCFRAMEWORK="${OUTPUT_DIR}/Giac.xcframework"

# Cleanup previous output
rm -rf "$OUTPUT_DIR"

# ---------- Step 1: Merge static libraries per architecture ----------

merge_libs() {
    local variant="$1"
    local builddir="$2"
    local prebuilt_dir="$3"
    local out_dir="${MERGED}/${variant}"

    mkdir -p "$out_dir"
    libtool -static -o "${out_dir}/Giac.a" \
        "${builddir}/libgiac.a" \
        "${builddir}/libsimpleinterface.a" \
        "${prebuilt_dir}/libgmp.a" \
        "${prebuilt_dir}/libmpfr.a"
}

echo "Merging static libraries..."

merge_libs "ios_arm64" \
    "${ROOT_DIR}/builddir-ios-arm64" \
    "${PREBUILT}/ios/arm64"

merge_libs "iphonesimulator_arm64" \
    "${ROOT_DIR}/builddir-ios-sim-arm64" \
    "${PREBUILT}/iphonesimulator/arm64"

merge_libs "iphonesimulator_x86_64" \
    "${ROOT_DIR}/builddir-ios-sim-x86_64" \
    "${PREBUILT}/iphonesimulator/x86_64"

merge_libs "maccatalyst_arm64" \
    "${ROOT_DIR}/builddir-maccatalyst-arm64" \
    "${PREBUILT}/maccatalyst/arm64"

merge_libs "maccatalyst_x86_64" \
    "${ROOT_DIR}/builddir-maccatalyst-x86_64" \
    "${PREBUILT}/maccatalyst/x86_64"

# ---------- Step 2: Create fat binaries with lipo ----------

echo "Creating fat binaries..."

mkdir -p "${MERGED}/iphonesimulator" "${MERGED}/maccatalyst" "${MERGED}/ios"

# iOS device (single arch, but still copy through lipo for consistency)
lipo -create \
    "${MERGED}/ios_arm64/Giac.a" \
    -output "${MERGED}/ios/Giac.a"

# iOS Simulator (arm64 + x86_64)
lipo -create \
    "${MERGED}/iphonesimulator_arm64/Giac.a" \
    "${MERGED}/iphonesimulator_x86_64/Giac.a" \
    -output "${MERGED}/iphonesimulator/Giac.a"

# Mac Catalyst (arm64 + x86_64)
lipo -create \
    "${MERGED}/maccatalyst_arm64/Giac.a" \
    "${MERGED}/maccatalyst_x86_64/Giac.a" \
    -output "${MERGED}/maccatalyst/Giac.a"

# ---------- Step 3: Create framework bundles ----------

create_framework() {
    local variant="$1"
    local lib_path="$2"
    local plist_name="${3:-Info.plist}"
    local out="${FRAMEWORKS}/${variant}/Giac.framework"

    mkdir -p "${out}/Headers" "${out}/Resources"
    cp "${lib_path}" "${out}/Giac"
    cp "${HEADERS}"/*.hpp "${out}/Headers/"
    if [ -f "${FRAMEWORK_SRC}/${plist_name}" ]; then
        cp "${FRAMEWORK_SRC}/${plist_name}" "${out}/Resources/Info.plist"
    fi
}

echo "Creating framework bundles..."

create_framework "ios" "${MERGED}/ios/Giac.a" "Info.plist"
create_framework "iphonesimulator" "${MERGED}/iphonesimulator/Giac.a" "framework.plist"
create_framework "maccatalyst" "${MERGED}/maccatalyst/Giac.a"

# ---------- Step 4: Assemble xcframework ----------

echo "Assembling Giac.xcframework..."

rm -rf "$XCFRAMEWORK"
mkdir -p "$XCFRAMEWORK"

# Copy framework variants into xcframework structure
cp -R "${FRAMEWORKS}/ios/Giac.framework" "${XCFRAMEWORK}/ios-arm64/Giac.framework"
mkdir -p "${XCFRAMEWORK}/ios-arm64"
cp -R "${FRAMEWORKS}/ios/Giac.framework" "${XCFRAMEWORK}/ios-arm64/Giac.framework"

mkdir -p "${XCFRAMEWORK}/ios-arm64_x86_64-simulator"
cp -R "${FRAMEWORKS}/iphonesimulator/Giac.framework" "${XCFRAMEWORK}/ios-arm64_x86_64-simulator/Giac.framework"

mkdir -p "${XCFRAMEWORK}/ios-arm64_x86_64-maccatalyst"
cp -R "${FRAMEWORKS}/maccatalyst/Giac.framework" "${XCFRAMEWORK}/ios-arm64_x86_64-maccatalyst/Giac.framework"

# Copy xcframework plist
cp "${FRAMEWORK_SRC}/xcframework.plist" "${XCFRAMEWORK}/Info.plist"

echo "Done: ${XCFRAMEWORK}"
