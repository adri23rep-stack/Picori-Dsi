#!/bin/sh
# E29.8: the existing 3DS port already translated ORIGINAL Thumb ROM
# function 0x08016A04. Extract its literal body, no new sprite rules.
set -eu
SRC=$1
DEST=$2
test -f "$SRC"
mkdir -p "$(dirname "$DEST")"
{
  printf '%s\n' '/* Generated unmodified from port/port_linked_stubs.c. */'
  printf '%s\n' '#include "entity.h"'
  awk '
    /^void UpdateSpriteForCollisionLayer\(Entity\* entity\) \{/ {active=1; found=1}
    active {print}
    active && /^}$/ {exit}
    END {if(!found) exit 1}
  ' "$SRC"
} > "$DEST.tmp"
mv "$DEST.tmp" "$DEST"
