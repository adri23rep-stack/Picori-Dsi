#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_collision_probe_e42.py" "$ROOT" "$TMP"
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
 ${E42_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -I"$ROOT/platform/dsi/source" \
 -include "$ROOT/include/region.h" \
 "$TMP/dsi_collision_probe_core_e42.c" "$ROOT/src/playerHitbox.c" "$ROOT/src/sineTable.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_probe_e42.c" \
 "$ROOT/platform/dsi-engine/source/dsi_rom_refs.c" "$HERE/engine_e42_probe_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
