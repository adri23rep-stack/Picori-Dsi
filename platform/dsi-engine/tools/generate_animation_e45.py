#!/usr/bin/env python3
"""Native metadata from original asset manifest/ASM, literal animation code."""
from pathlib import Path
import json
import re
import sys
root,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
assets={}
offsets={}
for entry in json.loads((root/'assets/assets.json').read_text()):
    if 'offsets' in entry: offsets=entry['offsets']
    if all(k in entry for k in ('path','start','size')):
        assets[entry['path']]=(entry['start']+offsets.get('EU',0),entry['size'])
asm=(root/'data/gfx/link_animations.s').read_text()
tables={}
for match in re.finditer(r'^(gSpriteAnimations_\w+):: @ ([0-9A-Fa-f]+)\n((?:\s*\.4byte[^\n]*\n)+)',asm,re.M):
    entries=re.findall(r'\.4byte\s+(\w+)',match[3])
    if entries[-1]!='00000000':raise SystemExit('Missing animation terminator')
    tables[match[1]]=(int(match[2],16)+0x98,len(entries)-1)
streams=[assets['animations/'+name+'.bin'] for name in
         re.findall(r'^\s*\.include "animations/(\w+)\.s"',asm,re.M)]
start=min(a for a,n in streams)
end=min(address-0x08000000 for address,count in tables.values())
if max(a+n for a,n in streams)>end:raise SystemExit('Animation data overlaps table')
gfx,gfx_size=assets['sprites/gSprite_Link.4bpp']
ptrs=(root/'data/gfx/sprite_ptrs.s').read_text().split('gSpritePtrs::',1)[1]
words=re.findall(r'^\s*\.4byte (\w+)',ptrs,re.M)
descriptors=[]
for sprite in [1,2,3,4,5,6,7,8,9,12]:
    animation,frames,tiles,pad=words[sprite*4:sprite*4+4]
    if tiles!='gSprite_Link' or pad!='00000000':raise SystemExit('Link sprite changed')
    addr,count=tables[animation]
    frame_addr,frame_size=assets['sprites/'+frames+'.bin']
    if frame_size%4 or frame_size//4>256 or count>256:raise SystemExit('Oversized sprite metadata')
    descriptors.append((sprite,addr,count,frame_addr+0x08000000,frame_size//4))
h=f"""/* Generated from original ASM and assets/assets.json; NO ROM bytes. */
#define E45_ANIM_ROM {hex(start+0x08000000)}u
#define E45_ANIM_BYTES {end-start}u
#define E45_GFX_ROM {hex(gfx+0x08000000)}u
#define E45_GFX_BYTES {gfx_size}u
#define E45_SPRITES {len(descriptors)}u
typedef struct {{ u16 sprite; u32 animations; u16 animCount; u32 frames; u16 frameCount; }} E45Metadata;
static const E45Metadata e45Metadata[E45_SPRITES]={{
"""
for sprite,addr,count,frames,n in descriptors:
    h+=f'    {{{sprite},{hex(addr)}u,{count},{hex(frames)}u,{n}}},\n'
h+='};\n'
(out/'dsi_animation_metadata_e45.h').write_text(h)
source=(root/'port/port_animation.c').read_text()
c="""/* Literal existing GBA-to-C animation translation, native data bindings. */
#include "entity.h"
#include "structures.h"
#include "dsi_animation_e45.h"
#include <stdio.h>
#define AnimRangeHasBytes DsiAnimationE45_RequireBytes
#define Port_GetSpritePtr DsiAnimationE45_Sprite
#define Port_GetSpriteAnimationData DsiAnimationE45_Data
int Port_FselAnimTrace;
unsigned long Port_UASF_Calls;
"""
for name in ['UpdateSpriteGfxSlot','FrameZero','InitializeAnimation',
             'UpdateAnimationVariableFrames','GetNextFrame','InitAnimationForceUpdate',
             'UpdateAnimationSingleFrame','sub_080042BA','sub_080042D0']:
    pattern=r'^(?:(?:static|inline|const) )*[A-Za-z_]\w* *\*? +'+name+r'\([^\n;]*\) \{\n.*?^}'
    match=re.search(pattern,source,re.M|re.S)
    if not match:raise SystemExit('Missing original '+name)
    c+=match.group()+'\n'
(out/'dsi_animation_core_e45.c').write_text(c)
