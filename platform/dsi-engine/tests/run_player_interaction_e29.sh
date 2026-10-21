#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
{
  printf '%s\n' '#include "player.h"' '#include "common.h"'
  awk '
    /^const InteractableObject gNoInteraction = \{/ {active=1; seen=1}
    active {print}
    active && /^};/ {exit}
    END {if (!seen) exit 1}
  ' "$ROOT/src/playerUtils.c"
  awk '
    /^void ResetPossibleInteraction\(void\) \{/ {active=1; seen=1}
    active {print}
    active && /^}$/ {exit}
    END {if (!seen) exit 1}
  ' "$ROOT/src/playerUtils.c"
} > "$TMP/original_player_interaction.c"
grep -q '^void ResetPossibleInteraction(void) {' "$TMP/original_player_interaction.c"
FLAGS='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS -w $INC -include "$ROOT/include/region.h" \
  "$TMP/original_player_interaction.c" \
  "$ROOT/platform/dsi-engine/source/dsi_player_state_e29.c" \
  "$HERE/engine_e29_player_interaction_test.c" \
  -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
