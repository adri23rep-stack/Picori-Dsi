#!/bin/sh
# Extract exact original entity list/priority functions from src/entity.c.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
{
cat <<'HEADERS'
#include "entity.h"
#include "room.h"
#include "game.h"
#include "common.h"
void InitDefaultPriority(Entity *e);
void SetEntityPriority(Entity *e, u32 prio);
HEADERS
for name in gPrioritiesDefault gPrioritiesKinstoneMenu; do
    awk -v sig="const u8 $name[]" 'index($0,sig)==1{active=1} active{print} active && /^};/{exit}' "$ROOT/src/entity.c"
done
for signature in \
    'void SetEntityPriority(Entity* ent, u32 prio) {' \
    'void InitDefaultPriority(Entity* e) {' \
    'void AppendEntityToList(Entity* entity, u32 listIndex) {'; do
    awk -v sig="$signature" 'index($0,sig)==1{active=1} active{print} active && /^}$/{exit}' "$ROOT/src/entity.c"
done
} >"$TMP/original.c"
for symbol in InitDefaultPriority SetEntityPriority AppendEntityToList; do
    grep -q "^void $symbol(" "$TMP/original.c"
done
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" "$TMP/original.c" \
  "$HERE/engine_e29_entity_list_test.c" -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
