#!/bin/sh
# Host-only execution of the UNMODIFIED Zelda LoadRoomEntity/LoadRoomEntityList
# with its REAL entity allocator, registration, duplicate lookup & pools.
# Unported path dependencies are test-only traps, NOT included in ARM9.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/platform/dsi-engine/tools/generate_entity_core_e29.sh" \
 "$ROOT/src/entity.c" "$TMP/dsi_entity_core_e29.c"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_allocator_e30.sh" \
 "$ROOT/src/entity.c" "$ROOT/src/room.c" \
 "$TMP/dsi_entity_allocator_core_e30.c" "$TMP/dsi_room_entity_register_core_e30.c"
sh "$ROOT/platform/dsi-engine/tools/generate_room_entity_core_e30.sh" \
 "$ROOT/src/room.c" "$TMP/dsi_room_entity_list_core_e30.c" \
 "$TMP/dsi_room_entity_spawn_core_e30.c" "$TMP/dsi_room_entity_params_core_e30.c"
grep -q 'void sub_0804AF0C(Entity\* ent, const EntityData\* dat)' "$TMP/dsi_room_entity_params_core_e30.c"
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E29_ENTITY_CORE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
 "$TMP/dsi_entity_core_e29.c" \
 "$TMP/dsi_entity_allocator_core_e30.c" \
 "$TMP/dsi_room_entity_register_core_e30.c" \
 "$TMP/dsi_room_entity_list_core_e30.c" \
 "$TMP/dsi_room_entity_spawn_core_e30.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_state_e29.c" \
 "$HERE/engine_e30_room_spawn_chain_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
