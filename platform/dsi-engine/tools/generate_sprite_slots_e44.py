#!/usr/bin/env python3
"""Select original sprite-slot allocation; no graphics upload or player dispatch."""
from pathlib import Path
import re
import sys

root, out = map(Path, sys.argv[1:3])
out.mkdir(parents=True, exist_ok=True)

def function(path, name):
    source = (root / path).read_text()
    pattern = r'^(?:(?:static|inline|const) )*[A-Za-z_]\w* *\*? +' + name + r'\([^\n;]*\) \{\n.*?^}'
    match = re.search(pattern, source, re.M | re.S)
    if not match:
        raise SystemExit("Missing original " + path + "::" + name)
    return match.group() + "\n"

c = """/* Generated literal engine functions; existing PC_PORT corrections retained. */
#include "common.h"
#include "structures.h"
#include "color.h"
#include "vram.h"
#include "physics.h"
#define PORT_GFX_COMPACTION 1
void ReserveGFXSlots(u32,u32,u32);
void SetGFXSlotStatus(GfxSlot*,u32);
void sub_080AE0C8(u32,Entity*,u32);
u32 FindFreeGFXSlots(u32);
void CleanUpGFXSlots(void);
u32 FindNextOccupiedGFXSlot(u32);
u32 FindFirstFreeGFXSlot(void);
void sub_080AE218(u32,u32);
void MoveGFXSlots(u32,u32);
"""
for name in ["ResetPalettes", "LoadSwapGFX", "sub_080AE0C8", "ReserveGFXSlots",
             "SetGFXSlotStatus", "FindFreeGFXSlots", "CleanUpGFXSlots",
             "sub_080AE218", "MoveGFXSlots", "FindNextOccupiedGFXSlot",
             "FindFirstFreeGFXSlot"]:
    c += function("src/vram.c", name)
for name in ["sub_0806FD8C", "sub_0806FDA0"]:
    c += function("src/physics.c", name)
(out / "dsi_sprite_slots_e44.c").write_text(c)
