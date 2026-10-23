#!/usr/bin/env python3
"""Extract genuine sprite side-table allocator + clear from src/physics.c."""
from pathlib import Path
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: generate_extra_sprite_e47.py ROOT OUTDIR")
root, out = map(Path, sys.argv[1:])
source = (root / "src/physics.c").read_text()

def original(name, signature):
    if source.count(signature) != 1:
        raise SystemExit("missing/ambiguous upstream " + name)
    first = source.index(signature)
    last = source.find("\n}\n", first)
    if last < 0:
        raise SystemExit("upstream function brace missing: " + name)
    return source[first:last + 3]

clear = original("sub_0806FD8C", "void sub_0806FD8C() {")
allocate = original("sub_0806FDA0", "bool32 sub_0806FDA0(Entity* this) {")
if "ARRAY_COUNT(gUnk_020000C0)" not in allocate or "spriteAnimation[2]" not in allocate:
    raise SystemExit("unexpected original extra sprite allocator")
prelude = """/* Native relocation of GBA extra-sprite side table. Originals below. */
#include "physics.h"
#include "color.h"
#include "player.h"
#include "common.h"
#include "structures.h"
#include "entity.h"

struct_gUnk_020000C0 gUnk_020000C0[0x30];
Entity* gPlayerClones[3];

"""
out.mkdir(parents=True, exist_ok=True)
(out / "dsi_original_extra_sprite_e47.c").write_text(prelude + clear + "\n" + allocate)
