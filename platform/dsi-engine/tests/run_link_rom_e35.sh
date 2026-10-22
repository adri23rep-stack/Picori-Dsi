#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections \
 ${E35_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -I"$ROOT" -I"$ROOT/platform/dsi/source" \
 "$ROOT/platform/dsi/source/tmc_rom_stream.c" \
 "$ROOT/platform/dsi/source/tmc_link_sprite.c" \
 "$HERE/engine_e35_link_draw_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
