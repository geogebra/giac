#!/usr/bin/env bash
# embed-wasm.sh — Post-process Emscripten output into a single embeddable JS file
#
# Usage: embed-wasm.sh <builddir>/giacggb <output-path>/giac.wasm.js
#
# Steps:
#   1. Base64-encode giacggb.wasm
#   2. Copy giacggb.js and replace the WASM filename with a data URI
#   3. Replace 'Module' with '__ggb__giac' (GeoGebra namespace)
#   4. Remove '"use asm";' directive

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "Usage: $0 <giacggb-path-without-ext> <output-js-path>" >&2
    exit 1
fi

INPUT_BASE="$1"
OUTPUT="$2"

WASM_FILE="${INPUT_BASE}.wasm"
JS_FILE="${INPUT_BASE}.js"

if [ ! -f "$WASM_FILE" ]; then
    echo "Error: WASM file not found: $WASM_FILE" >&2
    exit 1
fi

if [ ! -f "$JS_FILE" ]; then
    echo "Error: JS file not found: $JS_FILE" >&2
    exit 1
fi

# Base64-encode the WASM file (portable: macOS uses -b0, GNU uses -w0)
if base64 --help 2>&1 | grep -q '\-w'; then
    WASM_B64=$(base64 -w0 "$WASM_FILE")
else
    WASM_B64=$(base64 -b0 "$WASM_FILE" 2>/dev/null || base64 "$WASM_FILE" | tr -d '\n')
fi

# Process the JS file
sed \
    -e "s|giacggb.wasm|data:application/wasm;base64,${WASM_B64}|g" \
    -e 's|Module|__ggb__giac|g' \
    -e 's|"use asm";||g' \
    "$JS_FILE" > "$OUTPUT"

echo "Created: $OUTPUT"
