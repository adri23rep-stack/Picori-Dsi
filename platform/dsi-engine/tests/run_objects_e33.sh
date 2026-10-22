#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_core_e29.sh" "$ROOT/src/entity.c" "$TMP/core.c"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_allocator_e30.sh" "$ROOT/src/entity.c" "$ROOT/src/room.c" "$TMP/allocator.c" "$TMP/register.c"
FLAGS='-DE31_ROOM_TEST -std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E31_LIFECYCLE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
sh "$ROOT/platform/dsi-engine/tools/generate_sprite_layer_e29.sh" "$ROOT/port/port_linked_stubs.c" "$TMP/sprite_layer.c"
cc $FLAGS ${E33_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} -I"$TMP" -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" -include "$ROOT/include/region.h" \
 "$TMP/core.c" "$TMP/allocator.c" "$TMP/register.c" "$TMP/dsi_lifecycle_services_e31.c" \
 "$TMP/dsi_priority_e32.c" "$TMP/dsi_room_objects_e33.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_object_tick_e33.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_state_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_lifecycle_state_e31.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_state_e29.c" \
 "$ROOT/platform/dsi-engine/tests/engine_e31_lifecycle_test.c" \
 "$TMP/sprite_layer.c" "$HERE/engine_e33_object_test.c" \
 -Wl,--wrap=ExecuteScript -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
