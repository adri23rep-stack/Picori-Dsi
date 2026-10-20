#!/bin/sh
# E28: tests exact original function bodies from src/gameUtils.c (no copied game logic).
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
{
    printf '#include "area.h"\n#include "game.h"\n#include "flags.h"\n#include "room.h"\n#include "common.h"\n#include "fade.h"\n#include <stdio.h>\n#define AREA_TABLE_COUNT 0x90\nvoid InitAllRoomResInfo(void);\n'
    awk '/^void InitRoomTransition\(void\) \{/ { active=1 } active { print } active && /^}$/ { exit }' "$ROOT/src/gameUtils.c"
    awk '/^void InitRoom\(void\) \{/ { active=1 } active { print } active && /^}$/ { exit }' "$ROOT/src/gameUtils.c"
} >"$TMP/source.c"
# Never accept a silently empty extraction if the upstream function is renamed.
grep -q '^void InitRoomTransition(void) {' "$TMP/source.c"
grep -q '^void InitRoom(void) {' "$TMP/source.c"
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
${CC:-cc} $FLAGS -w $INC -include "$ROOT/include/region.h" "$TMP/source.c" "$ROOT/src/flags.c" "$ROOT/src/data/areaMetadata.c" "$HERE/engine_e28_original_initroom_test.c" -Wl,--gc-sections -o "$TMP/e28"
"$TMP/e28"
