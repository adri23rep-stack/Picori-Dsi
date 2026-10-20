#!/bin/sh
# E29.1: compile original InitializePlayer() unchanged from src/gameUtils.c.
# Host-only mocks isolate the hardware/entity-list services not yet ported.
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
#include "player.h"
#include "save.h"
#include "entity.h"
#include "item.h"
void RegisterPlayerHitbox(void);
void ResetPossibleInteraction(void);
HEADERS
awk '/^void InitializePlayer\(void\) \{/ { active=1 } active{print} active && /^}$/ {exit}' "$ROOT/src/gameUtils.c"
} >"$TMP/original.c"
grep -q '^void InitializePlayer(void) {' "$TMP/original.c"
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
  "$TMP/original.c" "$HERE/engine_e29_player_init_test.c" \
  -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
