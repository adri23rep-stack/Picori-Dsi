#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FLAGS='-std=gnu11 -O2 -Wall -Wextra -Werror -DPC_PORT -DTMC_DSI -DTMC_DSI_E28_ORIGINAL_INITROOM -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS $INC -include "$ROOT/include/region.h" \
  "$ROOT/platform/dsi-engine/source/dsi_player_preflight.c" \
  "$HERE/engine_e29_player_preflight_test.c" -o "$TMP/test"
"$TMP/test"
