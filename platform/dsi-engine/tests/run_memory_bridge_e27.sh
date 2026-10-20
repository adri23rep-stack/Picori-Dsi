#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
CC=${CC:-cc}
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
FLAGS="-std=gnu11 -O2 -ffunction-sections -fdata-sections -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0"
$CC $FLAGS -w $INC -include "$ROOT/include/region.h" \
  "$ROOT/src/data/areaMetadata.c" "$HERE/engine_e27_area_metadata_test.c" \
  -Wl,--gc-sections -o "$TMP/area"
"$TMP/area"
$CC -std=c11 -O2 -Wall -Wextra -Werror \
  -I"$ROOT/platform/dsi-engine/source" -I"$ROOT/platform/dsi/source" \
  "$ROOT/platform/dsi-engine/source/dsi_rom_refs.c" \
  "$HERE/engine_e27_memory_test.c" -o "$TMP/rom"
"$TMP/rom"
