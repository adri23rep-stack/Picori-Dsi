#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_animation_e45.py" "$ROOT" "$TMP"
cc -std=gnu11 -O1 -g -fno-strict-aliasing -fwrapv \
 ${E45_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DTMC_DSI_E45_ANIMATION -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -I"$ROOT/platform/dsi/source" -I"$TMP" \
 -include "$ROOT/include/region.h" "$TMP/dsi_animation_core_e45.c" \
 "$ROOT/platform/dsi-engine/source/dsi_animation_e45.c" \
 "$ROOT/platform/dsi-engine/source/dsi_rom_refs.c" "$HERE/engine_e45_animation_test.c" \
 -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test" "$@"
