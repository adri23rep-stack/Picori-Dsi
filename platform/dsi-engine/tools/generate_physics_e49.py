#!/usr/bin/env python3
"""Port existing physics functions literally, avoiding unrelated GBA ASM."""
from pathlib import Path
import sys
if len(sys.argv) != 3:
    raise SystemExit("usage: generate_physics_e49.py ROOT OUTDIR")
root, out = map(Path, sys.argv[1:])
physics = (root / "src/physics.c").read_text()
translated = (root / "port/port_linked_stubs.c").read_text()
def select(code, signature):
    if code.count(signature) != 1:
        raise SystemExit("ambiguous original source: " + signature)
    start = code.index(signature)
    end = code.find("\n}\n", start)
    if end < 0:
        raise SystemExit("missing end of " + signature)
    return code[start:end + 3]
body = ("/* Native exact upstream physics source / GBA ASM translations. */\n"
        '#include "physics.h"\n#include "entity.h"\n'
        '#include "global.h"\n'
        + select(physics, "s16 FixedDiv(s16 r0, s16 r1) {") + "\n"
        + select(physics, "void LinearMoveUpdate(Entity* ent) {") + "\n"
        + select(translated, "u32 GravityUpdate(Entity* entity, u32 gravity) {"))
out.mkdir(parents=True, exist_ok=True)
(out / "dsi_original_physics_e49.c").write_text(body)
