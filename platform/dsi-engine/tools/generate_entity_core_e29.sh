#!/bin/sh
# E29.4: original ARM9 code selected at build time from untouched src/entity.c.
set -eu
SRC=$1
DEST=$2
test -f "$SRC"
mkdir -p "$(dirname "$DEST")"
{
cat <<'PREFIX'
/* AUTO-GENERATED FROM ORIGINAL src/entity.c; DO NOT EDIT.
 * No Zelda logic copied or reimplemented. Only functions whose dependencies
 * have been audited are eligible for native ARM9 integration.
 */
#include "entity.h"
#include "room.h"
#include "game.h"
void SetEntityPriority(Entity*, u32);
void InitDefaultPriority(Entity*);
PREFIX
for array in gPrioritiesDefault gPrioritiesKinstoneMenu; do
  awk -v signature="const u8 $array[]" '
    index($0,signature)==1 {active=1;seen=1}
    active {print}
    active && /^};/ {exit}
    END {if(!seen)exit 1}
  ' "$SRC"
done
for fn in 'void SetEntityPriority(Entity* ent, u32 prio) {' \
          'void InitDefaultPriority(Entity* e) {' \
          'void SetInitializationPriority(void) {' \
          'void AppendEntityToList(Entity* entity, u32 listIndex) {'; do
  awk -v signature="$fn" '
    index($0,signature)==1 {active=1;seen=1}
    active {print}
    active && /^}$/ {exit}
    END {if(!seen)exit 1}
  ' "$SRC"
done
} >"$DEST.tmp"
mv "$DEST.tmp" "$DEST"
