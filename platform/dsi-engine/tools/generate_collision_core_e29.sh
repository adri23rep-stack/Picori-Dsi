#!/bin/sh
# E29.5: extract the ORIGINAL RegisterPlayerHitbox from src/collision.c.
# This is a build-time selection of authentic game code, NOT a DSi rewrite.
set -eu
SRC=$1
DEST=$2
test -f "$SRC"
mkdir -p "$(dirname "$DEST")"
{
cat <<'PREFIX'
/* Generated from original src/collision.c for ARM9: do not edit output.
 * Other collision routines are disabled pending porting the GBA hardware.
 */
#include "collision.h"
#include "player.h"
#include "common.h"
extern LinkedList2 gUnk_03003C70[16];
PREFIX
awk '
  /^void RegisterPlayerHitbox\(void\) \{/ { active=1; found=1 }
  active { print }
  active && /^}$/ { exit }
  END { if(!found) exit 1 }
' "$SRC"
} >"$DEST.tmp"
mv "$DEST.tmp" "$DEST"
