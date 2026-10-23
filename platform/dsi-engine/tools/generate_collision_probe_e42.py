#!/usr/bin/env python3
"""Select original port translations of GBA collision queries and hitbox probes."""
from pathlib import Path
import re,sys
root,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
s=(root/'port/port_linked_stubs.c').read_text()
def fn(name,source=s):
    m=re.search(r'^(?:(?:static|inline|const) )*[A-Za-z_]\w* *\*? +'+name+r'\([^\n;]*\) \{\n.*?^}',source,re.M|re.S)
    if not m:raise SystemExit('Missing original '+name)
    return m.group()+'\n'
c='''/* Existing translations selected verbatim, with native data bindings. */
#include "entity.h"
#include "player.h"
#include "room.h"
#include "map.h"
#include "physics.h"
#include "hitbox.h"
#include "functions.h"
#include "dsi_collision_probe_e42.h"
#define Port_GetCollisionShapeData DsiCollisionE42_Shape
#define port_resolve_addr DsiCollisionE42_Hitbox
'''
for i,n in enumerate(['082DC','0833C','0839C','083FC','0845C','084BC','0851C']):
    c+=f'#define gUnk_080{n} (gDsiE42CollisionParams[{i}])\n'
c+='static const u8* sActiveCollisionParams=gUnk_080082DC;\n'
# Decode the original assembler byte table as native halfwords, independent
# of host endianness/alignment. Never derive new movement masks.
asm=(root/'asm/src/code_08001A7C.s').read_text()
table=asm.split('gUnk_0800275C::',1)[1].split('thumb_func_start',1)[0]
values=[int(v,16) for v in re.findall(r'0x[0-9a-fA-F]+',table)]
if len(values)!=64 or any(v>255 for v in values):
    raise SystemExit('Invalid original direction masks')
c+='static const u16 gCollisionDirectionMasks[32]={'+','.join(
    hex(values[i]|values[i+1]<<8) for i in range(0,64,2))+'};\n'
c+=fn('FixedMul',(root/'src/physics.c').read_text())
for n in ['GetMapLayerForLayer','NormalizeTilePos','WorldToTilePos','RoomToTilePos','TilePosFromRoomTile',
          'GetCollisionDataAtTilePos','GetCollisionDataAtRoomTile','GetCollisionDataAtRoomCoords',
          'GetCollisionDataAtWorldCoords','GetCollisionDataAtEntity','GetCollisionDataRelativeTo',
          'SelectPlayerCollisionTable','TileCollisionLookup','sub_080085CC','sub_080085B0','sub_080086D8','sub_080086B4',
          'CalcCollisionDirectionOLD','LinearMoveDirectionOLD','sub_0800857C']:
    c+=fn(n)
# Native ARM9 already compiles playerUtils.c. Select the identical functions
# for host verification, with the same bindings and original direction table.
c+='#ifndef ARM9\n'
player=(root/'src/playerUtils.c').read_text()
table=re.search(r'^const u8 gUnk_0811C118\[\] = \{.*?^};',player,re.M|re.S)
if not table:raise SystemExit('Missing original slope directions')
c+=table.group()+'\n'
for n in ['sub_08079F48','sub_08079E90','sub_0807A750','sub_0807A5B8','UpdatePlayerMovement','sub_08079E58']:
    c+=fn(n,player)
c+='#endif\n'
c+='''
u32 DsiCollisionE42_OriginalPoint(Entity* e,u32 x,u32 y) {
    sActiveCollisionParams=SelectPlayerCollisionTable();
    return TileCollisionLookup(x,y,e);
}
'''
(out/'dsi_collision_probe_core_e42.c').write_text(c)
