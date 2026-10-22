#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
FLAGS='-std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
cc $FLAGS ${E35_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -include "$ROOT/include/region.h" \
 "$TMP/dsi_collision_lookup_e35.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_audit_e35.c" \
 "$HERE/engine_e35_collision_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
