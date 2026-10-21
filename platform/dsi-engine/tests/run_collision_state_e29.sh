#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/platform/dsi-engine/tools/generate_collision_core_e29.sh" \
  "$ROOT/src/collision.c" "$TMP/dsi_collision_core_e29.c"
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
  "$TMP/dsi_collision_core_e29.c" \
  "$ROOT/platform/dsi-engine/source/dsi_collision_state_e29.c" \
  "$HERE/engine_e29_collision_state_test.c" \
  -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
