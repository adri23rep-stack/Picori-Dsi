#!/usr/bin/env python3
"""Original GFX upload functions with native bounded DMA/LZ transport."""
from pathlib import Path
import re,sys
root,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
source=(root/'src/vram.c').read_text()
c="""/* Literal original upload scheduler and dirty-slot handling. */
#include "common.h"
#include "structures.h"
#include "color.h"
#include "dsi_sprite_upload_e46.h"
#define PORT_GFX_COMPACTION 1
#undef DmaCopy32
#define DmaCopy32(channel,source,destination,bytes) DsiSpriteUploadE46_Copy((uintptr_t)(source),(uintptr_t)(destination),(bytes))
#define LZ77UnCompVram DsiSpriteUploadE46_Lz
void sub_080ADE24(void);
void sub_080ADE74(u32);
"""
for name in ['sub_080ADD70','sub_080ADE24','sub_080ADE74']:
    match=re.search(r'^void '+name+r'\([^\n;]*\) \{\n.*?^}',source,re.M|re.S)
    if not match:raise SystemExit('Missing original '+name)
    c+=match.group()+'\n'
(out/'dsi_sprite_upload_core_e46.c').write_text(c)
