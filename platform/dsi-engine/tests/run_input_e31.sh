#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections \
 ${E31_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DTMC_DSI_E31_LIFECYCLE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -include "$ROOT/include/region.h" \
 "$TMP/dsi_lifecycle_services_e31.c" "$HERE/engine_e31_input_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
