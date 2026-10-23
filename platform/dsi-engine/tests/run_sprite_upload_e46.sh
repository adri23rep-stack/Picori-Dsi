#!/bin/sh
set -eu
ulimit -c 0
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_animation_e45.py" "$ROOT" "$TMP"
python3 "$ROOT/platform/dsi-engine/tools/generate_sprite_upload_e46.py" "$ROOT" "$TMP"
cc -std=gnu11 -O1 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
 ${E46_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DTMC_DSI_E45_ANIMATION -DTMC_DSI_E46_SPRITE_UPLOAD -DE46_UPLOAD_TEST \
 -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -I"$ROOT/platform/dsi/source" -I"$TMP" \
 -include "$ROOT/include/region.h" "$TMP/dsi_animation_core_e45.c" "$TMP/dsi_sprite_upload_core_e46.c" \
 "$ROOT/platform/dsi-engine/source/dsi_animation_e45.c" "$ROOT/platform/dsi-engine/source/dsi_sprite_upload_e46.c" \
 "$ROOT/platform/dsi-engine/source/dsi_rom_refs.c" "$ROOT/platform/dsi-engine/source/dsi_title_assets.c" \
 "$HERE/engine_e45_animation_test.c" -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test" "$@"
