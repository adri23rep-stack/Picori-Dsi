#!/usr/bin/env python3
"""Select original bodies, never generate replacement collision algorithms."""
from pathlib import Path
import re,sys
root,out=map(Path,sys.argv[1:3])
out.mkdir(parents=True,exist_ok=True)
def function(path,name):
    source=(root/path).read_text()
    match=re.search(r'^void '+name+r'\([^\n]*\) \{\n.*?^}',source,re.M|re.S)
    if not match:raise SystemExit('Missing original '+name)
    return match.group()+'\n'
source='''/* Exact original functions; ARM9 already links playerUtils.c. */
#include "map.h"
#include "room.h"
#include "collision.h"
extern u8 gMapTileTypeToCollisionData[];
extern u8 gMapSpecialTileToCollisionData[];
extern u8 gMapSpecialTileToActTile[];
'''
source+=function('src/scroll.c','FillActTileForLayer')
source+='\n#ifndef ARM9\n'
for name in ['sub_0807BBE4','CreateCollisionDataBorderAroundRoom']:
    source+=function('src/playerUtils.c',name)
source+='\n#endif\n'
(out/'dsi_collision_core_e41.c').write_text(source)
