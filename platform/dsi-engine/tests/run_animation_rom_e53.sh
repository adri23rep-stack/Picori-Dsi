#!/bin/sh
set -eu
ROOT=$(CDPATH='' cd -- "$(dirname "$0")/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
cc -std=gnu11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
 -DPC_PORT -DTMC_DSI -DNON_MATCHING -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
 -I"$ROOT/platform/dsi-engine/source" -I"$ROOT/platform/dsi/source" \
 "$ROOT/platform/dsi-engine/source/dsi_player_anim_rom_e53.c" \
 "$ROOT/platform/dsi-engine/tests/engine_e53_original_animation_rom_test.c" \
 -o "$TMP/test-e53"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test-e53"
echo "E53 native EU ROM sprite pointer/frame preflight passed under sanitizers"
