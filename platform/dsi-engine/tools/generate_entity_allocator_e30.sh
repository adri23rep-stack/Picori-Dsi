#!/bin/sh
# E30.5: take the literal ORIGINAL entity allocation/registration logic.
# No new entity allocator, registry or gameplay stubs in the ARM9 binary.
set -eu
ENTITY_SRC=$1
ROOM_SRC=$2
ALLOC=$3
REG=$4
test -f "$ENTITY_SRC"
test -f "$ROOM_SRC"
mkdir -p "$(dirname "$ALLOC")" "$(dirname "$REG")"
{
    printf '%s\n' '/* Generated ONLY from original src/entity.c; do not edit. */'
    # All upstream includes and the original PC_PORT pointer pool validator.
    # Stop BEFORE game-specific unrelated code; the full original routine
    # is extracted below without edits or changed safety rules.
    awk '{print} /^#include "tiles.h"$/ {seen=1;exit} END {if(!seen)exit 1}' "$ENTITY_SRC"
    printf '%s\n' 'extern UpdateContext gUpdateContext;'
    printf '%s\n' 'void ClearDeletedEntity(Entity*);'
    printf '%s\n' 'void DeleteEntity(Entity*);'
    awk '/^const u8 gEntityListLUT\[\] = \{/ {active=1;found=1} active {print} active && /^};/ {exit} END {if(!found)exit 1}' "$ENTITY_SRC"
    for FN in 'Entity* GetEmptyEntity() {' 'Manager* GetEmptyManager(void) {'; do
      awk -v signature="$FN" 'index($0, signature)==1 {active=1;found=1} active {print} active && /^}$/ {exit} END {if(!found)exit 1}' "$ENTITY_SRC"
    done
    awk '/^typedef void\* \(\*Getter\)\(void\);/ {print;found=1;exit} END {if(!found)exit 1}' "$ENTITY_SRC"
    awk '/^void\* GetEmptyEntityByKind\(u32 kind\) \{/ {active=1;found=1} active {print} active && /^}$/ {exit} END {if(!found)exit 1}' "$ENTITY_SRC"
} > "$ALLOC.tmp"
{
    printf '%s\n' '/* Generated original src/room.c::RegisterRoomEntity. */'
    printf '%s\n' '#include "room.h"'
    printf '%s\n' '#include "area.h"'
    printf '%s\n' '#include "common.h"'
    printf '%s\n' '#include "manager.h"'
    printf '%s\n' '#include "port/port_generic_entity.h"'
    printf '%s\n' 'extern const u8 gEntityListLUT[];'
    awk '/^void RegisterRoomEntity\(Entity\* ent, const EntityData\* dat\) \{/ {active=1;found=1} active {print} active && /^}$/ {exit} END {if(!found)exit 1}' "$ROOM_SRC"
} > "$REG.tmp"
mv "$ALLOC.tmp" "$ALLOC"
mv "$REG.tmp" "$REG"
