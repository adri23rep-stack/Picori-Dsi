#!/bin/sh
# E30.4 host execution of untouched ORIGINAL LoadRoomEntityList from src/room.c
# with genuine native RoomResInfo bank wiring. Only the unported spawn
# boundary is a test recorder; none is installed in a shipped .nds.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/platform/dsi-engine/tools/generate_room_entity_core_e30.sh" \
 "$ROOT/src/room.c" "$TMP/dsi_room_entity_list_core_e30.c" "$TMP/dsi_room_entity_spawn_core_e30.c"
grep -q '^void LoadRoomEntityList(const EntityData\* listPtr) {' "$TMP/dsi_room_entity_list_core_e30.c"
grep -q '^Entity\* LoadRoomEntity(const EntityData\* dat) {' "$TMP/dsi_room_entity_spawn_core_e30.c"
FLAGS='-std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS $INC -include "$ROOT/include/region.h" \
 "$TMP/dsi_room_entity_list_core_e30.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_property_native_e30.c" \
 "$HERE/engine_e30_original_room_entity_list_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
