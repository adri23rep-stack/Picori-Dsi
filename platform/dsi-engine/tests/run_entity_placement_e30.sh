#!/bin/sh
# Host tests of literal unmodified src/room.c::sub_0804AF0C branches.
# Test-only traps reject any unported GBA scripts/deletion, never shipped.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/platform/dsi-engine/tools/generate_room_entity_core_e30.sh" \
 "$ROOT/src/room.c" "$TMP/room_list.c" "$TMP/room_spawn.c" "$TMP/room_params.c"
FLAGS='-std=gnu11 -O2 -Wall -Wextra -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
 "$TMP/room_params.c" \
 "$HERE/engine_e30_entity_placement_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
