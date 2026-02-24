# Building Giac with Meson

Meson is an alternative build system for Giac, supporting native desktop builds
and cross-compilation to Android, iOS, Mac Catalyst, Windows, and WebAssembly.

## Prerequisites

- [Meson](https://mesonbuild.com/) >= 1.2.0
- [Ninja](https://ninja-build.org/) build backend
- C/C++ compiler (GCC, Clang, or MSVC)

## Quick Start: Build All Platforms

A convenience script builds every supported platform in one command:

```bash
./scripts/build-all.sh
```

This builds: native desktop, all 4 Android ABIs, all 5 iOS/Catalyst variants,
Windows (MinGW cross), and Emscripten WASM. See [Build All Script](#build-all-script)
for details and prerequisites.

## Native Desktop Builds

Native builds use system-installed GMP and MPFR libraries.

### Linux (Debian/Ubuntu)

```bash
sudo apt install meson ninja-build libgmp-dev libmpfr-dev

meson setup builddir
meson compile -C builddir
```

Outputs:
- `builddir/libgiac.a` -- static library
- `builddir/libjavagiac.so` -- JNI shared library (if JDK headers are available)
- `builddir/minigiac` -- standalone CAS shell

### macOS

```bash
brew install meson ninja gmp mpfr

meson setup builddir
meson compile -C builddir
```

Outputs:
- `builddir/libgiac.a` -- static library
- `builddir/libjavagiac.dylib` -- JNI shared library
- `builddir/minigiac` -- standalone CAS shell

### Windows (MSYS2 CLANG64)

```bash
pacman -S mingw-w64-clang-x86_64-meson mingw-w64-clang-x86_64-ninja \
          mingw-w64-clang-x86_64-gmp mingw-w64-clang-x86_64-mpfr

meson setup builddir
meson compile -C builddir
```

### Smoke Test

```bash
echo '1+1' | ./builddir/minigiac
# Expected output: 2
```

## Cross-Compilation

Cross-compilation uses prebuilt static GMP/MPFR libraries bundled in
`src/jni/prebuilt/`. Each target platform has a Meson cross-file in `cross/`.

### Android

Requires the [Android NDK](https://developer.android.com/ndk). The cross-files
ship with `@NDK@` placeholders — configure them before first use:

```bash
# Auto-detect NDK from ANDROID_NDK_HOME, ANDROID_HOME, or common locations
./scripts/configure-cross.sh

# Or specify explicitly
ANDROID_NDK_HOME=/path/to/ndk ./scripts/configure-cross.sh

# Build for a single ABI (use generated files from cross/local/)
meson setup builddir-android-arm64 --cross-file cross/local/android-arm64.ini
meson compile -C builddir-android-arm64

# Build for all 4 ABIs
for abi in arm arm64 x86 x86_64; do
  meson setup builddir-android-$abi --cross-file cross/local/android-$abi.ini
  meson compile -C builddir-android-$abi
done
```

#### `configure-cross.sh`

The Android cross-files in `cross/` are **templates** containing `@NDK@` and
`@NDK_HOST@` placeholders. The `scripts/configure-cross.sh` script replaces
these placeholders with your actual NDK path and generates ready-to-use files
in `cross/local/` (which is gitignored).

NDK detection order:
1. `ANDROID_NDK_HOME` environment variable
2. Latest version found in `$ANDROID_HOME/ndk/`
3. Common default locations:
   - macOS: `~/Library/Android/sdk/ndk/`
   - Linux: `~/Android/Sdk/ndk/`
   - CI: `/usr/local/lib/android/sdk/ndk/`

```bash
# Auto-detect
./scripts/configure-cross.sh

# Explicit path
ANDROID_NDK_HOME=/opt/android-ndk-r29 ./scripts/configure-cross.sh
```

The `build-all.sh` script calls `configure-cross.sh` automatically if
`cross/local/` does not exist yet.

Available cross-files (templates in `cross/`, generated in `cross/local/`):

| Template | ABI | Outputs |
|---|---|---|
| `android-arm.ini` | armeabi-v7a | `libgiac.a`, `libjavagiac.so` |
| `android-arm64.ini` | arm64-v8a | `libgiac.a`, `libjavagiac.so` |
| `android-x86.ini` | x86 | `libgiac.a`, `libjavagiac.so` |
| `android-x86_64.ini` | x86_64 | `libgiac.a`, `libjavagiac.so` |

### iOS / Mac Catalyst

Requires Xcode with iOS and macOS SDKs. The cross-files assume the default
Xcode SDK paths. Adjust the `sdk_path` constant if yours differ.

```bash
# Build for iOS device (arm64)
meson setup builddir-ios-arm64 --cross-file cross/ios-arm64.ini \
  -Dwith_simpleinterface=true
meson compile -C builddir-ios-arm64

# Build all 5 variants
for variant in ios-arm64 ios-sim-x86_64 ios-sim-arm64 \
               maccatalyst-x86_64 maccatalyst-arm64; do
  meson setup builddir-$variant --cross-file cross/$variant.ini \
    -Dwith_simpleinterface=true
  meson compile -C builddir-$variant
done

# Assemble into Giac.xcframework
./scripts/build-xcframework.sh
```

Available cross-files:

| Cross-file | Target | Outputs |
|---|---|---|
| `cross/ios-arm64.ini` | iOS device (arm64) | `libgiac.a`, `libsimpleinterface.a` |
| `cross/ios-sim-x86_64.ini` | iOS Simulator (x86_64) | `libgiac.a`, `libsimpleinterface.a` |
| `cross/ios-sim-arm64.ini` | iOS Simulator (arm64) | `libgiac.a`, `libsimpleinterface.a` |
| `cross/maccatalyst-x86_64.ini` | Mac Catalyst (x86_64) | `libgiac.a`, `libsimpleinterface.a` |
| `cross/maccatalyst-arm64.ini` | Mac Catalyst (arm64) | `libgiac.a`, `libsimpleinterface.a` |

### Windows (MinGW cross-compilation)

Requires the MinGW-w64 cross-compiler toolchain.

On macOS:
```bash
brew install mingw-w64
```

On Linux (Debian/Ubuntu):
```bash
sudo apt install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64
```

Build:
```bash
meson setup builddir-windows --cross-file cross/windows-clang64.ini
meson compile -C builddir-windows
```

Outputs:
- `builddir-windows/libgiac.a` -- static library
- `builddir-windows/libjavagiac.dll` -- JNI DLL (PE32+ x86-64)

> **Note:** The cross-file includes `-Wa,-mbig-obj` to handle `cocoa.cc`'s
> large number of PE/COFF sections from heavy template instantiation.

### WebAssembly (Emscripten)

Requires the [Emscripten SDK](https://emscripten.org/).

On macOS:
```bash
brew install emscripten
```

Or manually:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source emsdk_env.sh
```

Build:
```bash
meson setup builddir-wasm --cross-file cross/emscripten-wasm32.ini
meson compile -C builddir-wasm
```

Outputs:
- `builddir-wasm/giacggb.wasm` -- WebAssembly binary (~9.5 MB)
- `builddir-wasm/giacggb.js` -- Emscripten JS glue (~178 KB)
- `builddir-wasm/giac.wasm.js` -- single embeddable JS file (WASM inlined as base64, ~13 MB)

Test in Node.js:

```bash
node -e "
  const Module = require('./builddir-wasm/giacggb.js');
  Module.onRuntimeInitialized = () => {
    const caseval = Module.cwrap('caseval', 'string', ['string']);
    console.log(caseval('1+1'));
  };
"
# Expected output: 2
```

### Linux ARM (Raspberry Pi)

Requires an ARM cross-compilation toolchain (`arm-linux-gnueabihf-gcc`).

```bash
meson setup builddir-linux-arm --cross-file cross/linux-arm.ini
meson compile -C builddir-linux-arm
```

## Build All Script

The `scripts/build-all.sh` script automates building for every supported
platform from a macOS host. It builds targets sequentially and reports a
summary at the end.

### Prerequisites

The script checks for required tools before starting. Install them with:

```bash
# Core build tools
brew install meson ninja gmp mpfr

# Cross-compilation toolchains
brew install mingw-w64 emscripten

# Android NDK (set path in cross/android-*.ini files)
# iOS/Catalyst (requires Xcode)
```

### Usage

```bash
# Build all platforms
./scripts/build-all.sh

# Build specific platforms only
./scripts/build-all.sh native android

# Build with verbose output
./scripts/build-all.sh --verbose

# Clean all build directories first
./scripts/build-all.sh --clean
```

Available platform selectors: `native`, `android`, `ios`, `windows`, `wasm`.

### Output Summary

The script prints a summary table showing pass/fail status for each target,
along with the build artifacts produced.

## Build Options

Options are set at configure time with `-D`:

| Option | Type | Default | Description |
|---|---|---|---|
| `giac_version` | string | `1.2.4` | Version string embedded via `-DVERSION` |
| `with_jni` | feature | `auto` | Build JNI shared library (`auto` = build if not iOS/WASM) |
| `with_minigiac` | boolean | `true` | Build minigiac executable (native desktop only) |
| `with_simpleinterface` | boolean | `false` | Build SimpleInterface static library (iOS/Catalyst) |
| `with_wasm_embed` | boolean | `true` | Post-process WASM into single embedded JS file |

Examples:

```bash
# Disable JNI
meson setup builddir -Dwith_jni=disabled

# Custom version string
meson setup builddir -Dgiac_version=1.9.0-99

# iOS with SimpleInterface
meson setup builddir-ios --cross-file cross/ios-arm64.ini \
  -Dwith_simpleinterface=true
```

## Project Structure

```
meson.build              # Main build definition
meson_options.txt        # Build options
cross/                   # Cross-compilation files
  android-arm.ini
  android-arm64.ini
  android-x86.ini
  android-x86_64.ini
  emscripten-wasm32.ini
  ios-arm64.ini
  ios-sim-arm64.ini
  ios-sim-x86_64.ini
  linux-arm.ini
  maccatalyst-arm64.ini
  maccatalyst-x86_64.ini
  windows-clang64.ini
scripts/
  build-all.sh           # Build all platforms
  configure-cross.sh     # Generate Android cross-files from NDK path
  embed-wasm.sh          # WASM base64 post-processing
  build-xcframework.sh   # iOS xcframework assembly
```

## Notes

- Cross-compilation uses prebuilt static GMP 6.3.0 and MPFR 4.2.1 from
  `src/jni/prebuilt/` (Android, iOS, Linux ARM, Windows) or
  `src/giac.js/prebuilt/` (Emscripten).
- Windows cross-compilation requires `-Wa,-mbig-obj` due to `cocoa.cc`'s
  heavy template usage exceeding the PE/COFF default section limit.
- The Emscripten embed step uses Python 3 to handle base64-encoded WASM
  strings that exceed shell argument length limits.
- The Node.js addon (`binding.gyp`) is not managed by Meson and remains
  unchanged.
- This build system coexists with the existing CMake and Gradle builds.
