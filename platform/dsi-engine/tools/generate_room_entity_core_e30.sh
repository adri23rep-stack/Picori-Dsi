#!/bin/sh
# Extract two *untouched original function bodies* from src/room.c,
# which is the real engine source for both list dispatch and spawning.
# ARM9 compiles, but refuses to link/execute gameplay until the original
# allocator, rooms, entities, script resolver and graphics dependencies
# are ported. No fake C entity loader is introduced.
set -eu
SRC=$1
LIST=$2
SPAWN=$3
test -f "$SRC"
mkdir -p "$(dirname "$LIST")" "$(dirname "$SPAWN")"
{
cat <<'HEAD'
/* Original src/room.c::LoadRoomEntityList, extracted at build time. */
#include "room.h"
#include "port/port_region_data.h"
HEAD
awk '
  /^void LoadRoomEntityList\(const EntityData\* listPtr\) \{/ {active=1;found=1}
  active {print}
  active && /^}$/ {exit}
  END {if(!found) exit 1}
' "$SRC"
} > "$LIST.tmp"
{
cat <<'HEAD'
/* Original src/room.c::LoadRoomEntity, extracted at build time.
 * This part must not be invoked until the native dependency closure exists. */
#include "room.h"
#include "area.h"
#include "common.h"
#include "enemy.h"
#include "flags.h"
#include "script.h"
#include "game.h"
#include "manager/templeOfDropletsManager.h"
#include "manager/angryStatueManager.h"
#include "port/port_generic_entity.h"
#include "port/port_region_data.h"
extern Entity* GetEmptyEntityByKind(u32 kind);
void RegisterRoomEntity(Entity*, const EntityData*);
void sub_0804AF0C(Entity*, const EntityData*);
HEAD
awk '
  /^Entity\* LoadRoomEntity\(const EntityData\* dat\) \{/ {active=1;found=1}
  active {print}
  active && /^}$/ {exit}
  END {if(!found) exit 1}
' "$SRC"
} > "$SPAWN.tmp"
mv "$LIST.tmp" "$LIST"
mv "$SPAWN.tmp" "$SPAWN"
