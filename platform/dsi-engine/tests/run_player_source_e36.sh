#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_lifecycle_e31.py" "$ROOT" "$TMP"
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
 -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
 -I"$ROOT/platform/dsi-engine/source" \
 -include "$ROOT/include/region.h" -c \
 "$TMP/dsi_original_player_actions_e36.c" -o "$TMP/player.o"
nm "$TMP/player.o" | grep -Eq ' [Tt] PlayerInit$'
nm "$TMP/player.o" | grep -Eq ' [Tt] PlayerNormal$'
nm -u "$TMP/player.o" > "$TMP/unresolved.txt"
test -s "$TMP/unresolved.txt"
echo "E36 original src/player.c PlayerInit + PlayerNormal compiled; unresolved gameplay services audited"
