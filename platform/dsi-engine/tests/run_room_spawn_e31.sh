#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
ROM=${1:?Usage: run_room_spawn_e31.sh /path/to/EU.gba}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
python3 "$ROOT/platform/dsi-engine/tools/generate_collision_e41.py" "$ROOT" "$TMP"
python3 "$ROOT/platform/dsi-engine/tools/generate_collision_probe_e42.py" "$ROOT" "$TMP"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_core_e29.sh" "$ROOT/src/entity.c" "$TMP/core.c"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_allocator_e30.sh" "$ROOT/src/entity.c" "$ROOT/src/room.c" "$TMP/allocator.c" "$TMP/register.c"
FLAGS='-DE31_ROOM_TEST -std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E31_LIFECYCLE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
sh "$ROOT/platform/dsi-engine/tools/generate_room_entity_core_e30.sh" "$ROOT/src/room.c" "$TMP/list.c" "$TMP/spawn.c" "$TMP/params.c"
sh "$ROOT/platform/dsi-engine/tools/generate_sprite_layer_e29.sh" "$ROOT/port/port_linked_stubs.c" "$TMP/sprite.c"
cc $FLAGS ${E31_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} -I"$TMP" -I"$ROOT/platform/dsi/source" -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -include "$ROOT/include/region.h" \
 "$TMP/core.c" "$TMP/allocator.c" "$TMP/register.c" "$TMP/dsi_lifecycle_services_e31.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_state_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_lifecycle_state_e31.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_state_e29.c" \
 "$TMP/list.c" "$TMP/spawn.c" "$TMP/params.c" "$TMP/dsi_layer_resolve_e31.c" "$TMP/sprite.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_spawn_e31.c" \
 "$TMP/dsi_collision_core_e41.c" \
 "$TMP/dsi_collision_probe_core_e42.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_probe_e42.c" "$ROOT/src/playerHitbox.c" "$ROOT/src/sineTable.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_collision_e36.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_collision_e41.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_property_native_e30.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_property_data_e30.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_properties_e30.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_lists.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_probe.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_maps.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_tiles.c" \
 "$ROOT/platform/dsi-engine/source/dsi_title_assets.c" \
 "$ROOT/platform/dsi-engine/source/dsi_rom_refs.c" \
 "$ROOT/platform/dsi-engine/source/dsi_gba_bus.c" \
 "$ROOT/platform/dsi/source/tmc_rom_stream.c" \
 "$HERE/engine_e31_lifecycle_test.c" "$HERE/engine_e31_room_spawn_test.c" -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test" "$ROM"
