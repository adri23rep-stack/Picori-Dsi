#!/usr/bin/env python3
"""Select literal original function bodies; fail if source signatures drift."""
from pathlib import Path
import re, sys
root, out = map(Path, sys.argv[1:3])
out.mkdir(parents=True, exist_ok=True)
def fn(path, name):
    s=(root/path).read_text()
    m=re.search(r'^[^\n;{}]*\b'+re.escape(name)+r'\([^\n;]*\) \{\n.*?^}',s,re.M|re.S)
    if not m: raise SystemExit('Missing original function: '+path+'::'+name)
    return m.group()+'\n'
def array(path,name):
    s=(root/path).read_text()
    m=re.search(r'^(?:static )?const [^\n]*\b'+name+r'\[\] = \{.*?^};',s,re.M|re.S)
    if not m: raise SystemExit('Missing original array: '+name)
    return m.group()+'\n'
headers='''/* Generated from existing engine sources. Do not hand-edit. */
#include "entity.h"
#include "area.h"
#include "common.h"
#include "player.h"
#include "game.h"
#include "room.h"
#include "color.h"
#include "collision.h"
#include "script.h"
#include "enemy.h"
#include "vram.h"
#include "tiles.h"
#include "map.h"
#include "port_entity_ctx.h"
#include <string.h>
#include <stdio.h>
'''
s=headers+'''
extern LinkedList2 gUnk_03003C70[16];
extern ScriptExecutionContext gScriptExecutionContextArray[32];
extern ScriptExecutionContext gPlayerScriptExecutionContext;
void InitScriptExecutionContext(ScriptExecutionContext*, Script*);
u32 ConvInputToState(u32);
'''
s+=array('src/code_0805EC04.c','gUnk_08109202')
for p,names in {
 'src/vram.c':['UnloadGFXSlots'],
 'src/color.c':['UnloadOBJPalette','sub_0801D244'],
 'src/physics.c':['UnloadOBJPalette2','UnloadHitbox'],
 'src/code_0805EC04.c':['sub_0805EC60','InitPlayerMacro','UpdatePlayerInput','ConvInputToState'],
 'src/collision.c':['sub_08017744'],
 'src/enemyUtils.c':['Port_IsEntityInPool','EnemyDetachFX'],
 'src/script.c':['InitScriptData','CreateScriptExecutionContext','DestroyScriptExecutionContext','StartCutscene','InitScriptForEntity','UnloadCutsceneData','InitScriptExecutionContext'],
}.items():
 for name in names:s+=fn(p,name)
s+='\n#ifndef ARM9 /* ARM9 links these from original common/playerUtils objects. */\nextern u8 gzHeap[0x1000];\n#define gUnk_02035542 (gzHeap + 2)\n'
for p,names in {'src/common.c':['zMalloc','zFree','zMallocInit'], 'src/playerUtils.c':['GetInteractableObjectIndex','RemoveInteractableObject','FreeCarryEntity','sub_0807C8B0']}.items():
 for name in names:s+=fn(p,name)
s+='\n#endif\n'
(out/'dsi_lifecycle_services_e31.c').write_text(s)
# Include entity.c's real Temp definition/pool/validator from the allocator object.
s='''/* Included in the original allocator translation unit. */
static void UnlinkEntity(Entity*);
static void ReleaseTransitionManager(void*);
void DeleteAllEntities(void);
void ClearAllDeletedEntities(void);
void sub_0805E98C(void);
extern void UnloadOBJPalette2(Entity*);
extern void sub_08017744(Entity*);
extern void EnemyDetachFX(Entity*);
typedef void (*Deleter)(void*);
'''
for n in ['DeleteEntityAny','DeleteEntity','ClearAllDeletedEntities','ClearDeletedEntity','DeleteAllEntities','DeleteManager','ReleaseTransitionManager','UnlinkEntity','sub_0805E98C','EraseAllEntities','RecycleEntities','DeleteSleepingEntities']:
 s+=fn('src/entity.c',n)
(out/'dsi_lifecycle_entity_e31.inc').write_text(s)
# Reuse the already translated original ARM tile/layer routines.
s=headers
for n in ['GetMapLayerForLayer','NormalizeTilePos','WorldToTilePos','GetTileTypeAtTilePos','GetTileTypeAtWorldCoords']:
 s+=fn('port/port_linked_stubs.c',n)
s+='typedef struct { u16 actTile; u8 preservedLayer; u8 destinationLayer; } TransitionTileEntry;\n'
s+=array('port/port_linked_stubs.c','sResolveCollisionLayerTiles')
s+=fn('port/port_linked_stubs.c','GetActTileForTileType')
s+=fn('port/port_linked_stubs.c','ResolveCollisionLayer')
s+=array('src/collision.c','gMapTileTypeToActTile')
s+=fn('src/collision.c','GetMapTileTypeToActTile')
(out/'dsi_layer_resolve_e31.c').write_text(s)

# E32: extract the engine's literal entity priority rules.
priority='''/* Generated from ORIGINAL entity.c, no rewritten rules. */
#include "entity.h"
#include "player.h"
#include "message.h"
static void UpdatePriorityTimer(void);
'''
for name in [
 'EntityDisabled','AnyPrioritySet','SetMinPriority','ResetEntityPriority',
 'RequestPriority','RequestPriorityDuration','SetPriorityTimer',
 'UpdatePriorityTimer','UpdatePriority','SetPlayerEventPriority',
 'ResetPlayerEventPriority','RevokePriority','SetRoomReloadPriority',
 'ClearEventPriority',
]:
 priority+=fn('src/entity.c',name)
# Call through to original static frame priority update. No Zelda dispatcher.
priority+='\nvoid DsiPriorityE32_Tick(void) { UpdatePriority(); }\n'
(out/'dsi_priority_e32.c').write_text(priority)

# E33: selected exact bodies from ORIGINAL object/physics source, no new AI.
obj='''/* Original engine object, allocation and placement functions. */
#include "entity.h"
#include "object.h"
#include "physics.h"
#include "script.h"
'''
bed=(root/'src/object/bedCover.c').read_text()
anchor='\nvoid BedCover(BedCoverEntity* this) {'
if anchor not in bed: raise SystemExit('Original BedCover layout changed')
obj+=bed[:bed.index(anchor)]+'\n'
# The title renderer still owns its old title-only CreateObject symbol.
# Rename only the exported symbol of this EXACT original object allocator,
# preserving its original body and calls in native gameplay.
obj+='#define CreateObject DsiRoomE33_CreateObject\n'
for path,names in {
 'src/objectUtils.c':['CreateObject','CreateObjectWithParent'],
 'src/physics.c':['PositionRelative','CopyPosition'],
 'src/object/bedCover.c':['BedCover','sub_080928C8','nullsub_119','sub_08092930','sub_08092940','sub_08092958'],
}.items():
 for n in names: obj+=fn(path,n)
(out/'dsi_room_objects_e33.c').write_text(obj)

# E35: original Thumb-to-C collision tile lookup port, extracted literally.
collision='''/* Original port/port_linked_stubs.c GBA ARM collision translations. */
#include "entity.h"
#include "room.h"
#include "map.h"
'''
for name in ['GetMapLayerForLayer','NormalizeTilePos','WorldToTilePos']:
 collision+=fn('port/port_linked_stubs.c',name)
for name in ['GetCollisionDataAtTilePos','GetCollisionDataAtWorldCoords',
             'GetCollisionDataAtEntity','GetCollisionDataRelativeTo']:
 collision+=fn('port/port_linked_stubs.c',name)
(out/'dsi_collision_lookup_e35.c').write_text(collision)
