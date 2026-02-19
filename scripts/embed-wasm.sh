#!/usr/bin/env bash
# embed-wasm.sh — Post-process Emscripten output into a single embeddable JS file
#
# Usage: embed-wasm.sh <builddir>/giacggb[.js] <output-path>/giac.wasm.js
#
# Steps:
#   1. Base64-encode giacggb.wasm
#   2. Copy giacggb.js and replace the WASM filename with a data URI
#   3. Replace 'Module' with '__ggb__giac' (GeoGebra namespace)
#   4. Remove '"use asm";' directive

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "Usage: $0 <giacggb-path> <output-js-path>" >&2
    exit 1
fi

INPUT_BASE="${1%.js}"
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

# Base64-encode the WASM file to a temp file (avoids argument-length limits)
B64_FILE=$(mktemp)
trap 'rm -f "$B64_FILE"' EXIT

if base64 --help 2>&1 | grep -q '\-w'; then
    base64 -w0 "$WASM_FILE" > "$B64_FILE"
else
    base64 -i "$WASM_FILE" -b0 2>/dev/null > "$B64_FILE" || base64 < "$WASM_FILE" | tr -d '\n' > "$B64_FILE"
fi

# Use Python3 to do the replacements (handles arbitrarily large strings)
python3 - "$JS_FILE" "$B64_FILE" "$OUTPUT" << 'PYEOF'
import sys

js_path, b64_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]

with open(js_path, 'r') as f:
    js = f.read()
with open(b64_path, 'r') as f:
    b64 = f.read()

js = js.replace('giacggb.wasm', 'data:application/wasm;base64,' + b64)
js = js.replace('Module', '__ggb__giac')
js = js.replace('"use asm";', '')

with open(out_path, 'w') as f:
    f.write(js)
PYEOF

echo "Created: $OUTPUT"
