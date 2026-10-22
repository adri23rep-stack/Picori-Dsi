#!/usr/bin/env python3
"""Select original port translations of GBA collision queries and hitbox probes."""
from pathlib import Path
import re,sys
root,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
s=(root/'port/port_linked_stubs.c').read_text()
def fn(name):
    m=re.search(r'^[^\n;{}]*\b'+name+r'\([^\n;]*\) \{\n.*?^}',s,re.M|re.S)
    if not m:raise SystemExit('Missing original '+name)
    return m.group()+'\n'
c='''/* Existing translations selected verbatim, with native data bindings. */
#include "entity.h"
#include "player.h"
#include "room.h"
#include "map.h"
#include "dsi_collision_probe_e42.h"
#define Port_GetCollisionShapeData DsiCollisionE42_Shape
#define port_resolve_addr DsiCollisionE42_Hitbox
'''
for i,n in enumerate(['082DC','0833C','0839C','083FC','0845C','084BC','0851C']):
    c+=f'#define gUnk_080{n} (gDsiE42CollisionParams[{i}])\n'
c+='static const u8* sActiveCollisionParams=gUnk_080082DC;\n'
for n in ['GetMapLayerForLayer','NormalizeTilePos','WorldToTilePos','RoomToTilePos','TilePosFromRoomTile',
          'GetCollisionDataAtTilePos','GetCollisionDataAtRoomTile','GetCollisionDataAtRoomCoords',
          'GetCollisionDataAtWorldCoords','GetCollisionDataAtEntity','GetCollisionDataRelativeTo',
          'SelectPlayerCollisionTable','TileCollisionLookup','sub_080085CC','sub_080085B0','sub_080086D8','sub_080086B4']:
    c+=fn(n)
c+='''
u32 DsiCollisionE42_OriginalPoint(Entity* e,u32 x,u32 y) {
    sActiveCollisionParams=SelectPlayerCollisionTable();
    return TileCollisionLookup(x,y,e);
}
'''
(out/'dsi_collision_probe_core_e42.c').write_text(c)
