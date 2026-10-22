#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FLAGS='-std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E31_LIFECYCLE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/platform/dsi-engine/source"
cc $FLAGS ${E39_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} $INC -include "$ROOT/include/region.h" \
 "$ROOT/platform/dsi-engine/source/dsi_room_link_origin_e39.c" \
 "$HERE/engine_e39_link_origin_test.c" -Wl,--gc-sections -o "$TMP/link"
ASAN_OPTIONS=detect_leaks=0 "$TMP/link"
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_core_e29.sh" "$ROOT/src/entity.c" "$TMP/core.c"
sh "$ROOT/platform/dsi-engine/tools/generate_entity_allocator_e30.sh" "$ROOT/src/entity.c" "$ROOT/src/room.c" "$TMP/allocator.c" "$TMP/register.c"
sh "$ROOT/platform/dsi-engine/tools/generate_sprite_layer_e29.sh" "$ROOT/port/port_linked_stubs.c" "$TMP/sprite_layer.c"
python3 - "$ROOT" "$TMP" <<'PY'
from pathlib import Path
import re,sys
root,tmp=map(Path,sys.argv[1:3])
s=(root/'src/gameUtils.c').read_text()
m=re.search(r'^bool32 AreaIsDungeon\(void\) \{\n.*?^\}',s,re.M|re.S)
if not m:raise SystemExit('Original AreaIsDungeon not found')
(tmp/'area.c').write_text('#include "area.h"\n'+m.group()+'\n')
PY
cc -DE31_ROOM_TEST $FLAGS ${E39_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} $INC -I"$TMP" -include "$ROOT/include/region.h" \
 "$TMP/core.c" "$TMP/allocator.c" "$TMP/register.c" "$TMP/dsi_lifecycle_services_e31.c" \
 "$TMP/dsi_priority_e32.c" "$TMP/area.c" "$ROOT/src/object/archway.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_archway_e39.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_state_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_lifecycle_state_e31.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_state_e29.c" \
 "$ROOT/platform/dsi-engine/tests/engine_e31_lifecycle_test.c" \
 "$TMP/sprite_layer.c" "$HERE/engine_e39_archway_test.c" \
 -Wl,--gc-sections -o "$TMP/arch"
ASAN_OPTIONS=detect_leaks=0 "$TMP/arch"
