#!/bin/sh
# E28.4: compile the ORIGINAL room-init function bodies from src/gameUtils.c.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
{
cat <<'HEADERS'
#include "area.h"
#include "game.h"
#include "flags.h"
#include "room.h"
#include "common.h"
#include "fade.h"
#include "dsi_area_headers.h"
#include "dsi_room_resources.h"
#include <stdio.h>
#define AREA_TABLE_COUNT 0x90
void InitRoomTransition(void);
void InitAllRoomResInfo(void);
void InitRoomResInfo(RoomResInfo*, RoomHeader*, u32, u32);
RoomResInfo* GetCurrentRoomInfo(void);
RoomHeader* GetAreaRoomHeaderTable(u32);
HEADERS
for signature in \
  'void InitRoomTransition(void) {' \
  'void InitRoom(void) {' \
  'void InitAllRoomResInfo(void) {' \
  'void InitRoomResInfo(' \
  'RoomResInfo* GetCurrentRoomInfo(void) {'; do
    awk -v sig="$signature" 'index($0,sig)==1 { active=1 } active{print} active && /^}$/ {exit}' \
        "$ROOT/src/gameUtils.c"
done
} >"$TMP/original.c"
for symbol in InitRoomTransition InitRoom InitAllRoomResInfo InitRoomResInfo GetCurrentRoomInfo; do
  grep -Eq "^(void|RoomResInfo\*) $symbol[(]" "$TMP/original.c"
done
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E28_ORIGINAL_INITROOM -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
  "$TMP/original.c" "$ROOT/src/data/areaMetadata.c" "$ROOT/src/flags.c" \
  "$HERE/engine_e28_full_room_init_test.c" -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
