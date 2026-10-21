#!/bin/sh
# E29.7: host-only integration of four untouched ORIGINAL source routines:
# InitializePlayer (gameUtils), AppendEntityToList (entity), 
# RegisterPlayerHitbox (collision), ResetPossibleInteraction (playerUtils).
# The ONLY gameplay-sensitive mock is the unported sprite-layer hardware
# boundary, explicitly asserted to have been called. This test does not
# purport to make Link native or screen-rendered on DSi.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
sh "$ROOT/platform/dsi-engine/tools/generate_entity_core_e29.sh" \
    "$ROOT/src/entity.c" "$TMP/dsi_entity_core_e29.c"
sh "$ROOT/platform/dsi-engine/tools/generate_collision_core_e29.sh" \
    "$ROOT/src/collision.c" "$TMP/dsi_collision_core_e29.c"
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
awk '
  /^void InitializePlayer\(void\) \{/ { active=1; found=1 }
  active { print }
  active && /^}$/ { exit }
  END { if(!found) exit 1 }
' "$ROOT/src/gameUtils.c"
} > "$TMP/original_player.c"
{
  printf '%s\n' '#include "player.h"' '#include "common.h"'
  awk '
    /^const InteractableObject gNoInteraction = \{/ {active=1; found=1}
    active {print}
    active && /^};/ {exit}
    END {if(!found) exit 1}
  ' "$ROOT/src/playerUtils.c"
  awk '
    /^void ResetPossibleInteraction\(void\) \{/ {active=1; found=1}
    active {print}
    active && /^}$/ {exit}
    END {if(!found) exit 1}
  ' "$ROOT/src/playerUtils.c"
} > "$TMP/original_interaction.c"
for symbol in InitializePlayer ResetPossibleInteraction RegisterPlayerHitbox AppendEntityToList; do
    grep -q "$symbol" "$TMP/original_player.c" "$TMP/original_interaction.c" \
        "$TMP/dsi_entity_core_e29.c" "$TMP/dsi_collision_core_e29.c"
done
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
 "$TMP/original_player.c" "$TMP/original_interaction.c" \
 "$TMP/dsi_entity_core_e29.c" "$TMP/dsi_collision_core_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_entity_state_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_collision_state_e29.c" \
 "$ROOT/platform/dsi-engine/source/dsi_player_state_e29.c" \
 "$HERE/engine_e29_player_chain_test.c" -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
