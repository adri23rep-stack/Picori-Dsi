#!/usr/bin/env python3
"""E50: select exact original translated tile and surface query functions."""
from pathlib import Path
import re, sys

if len(sys.argv) != 3:
    raise SystemExit("usage: generate_tile_queries_e50.py ROOT OUT")
root, out = map(Path, sys.argv[1:])
out.mkdir(parents=True, exist_ok=True)

def select(path, name):
    source = (root / path).read_text()
    pattern = r'^((?:static inline |static )?(?:MapLayer\*|u32|u16|s32|bool32|void) +[^\n;{}]*\b' + re.escape(name) + r'\([^\n;]*\) \{\n.*?^})'
    matches = re.findall(pattern, source, re.M | re.S)
    if len(matches) != 1:
        raise SystemExit("Ambiguous/missing original: " + path + "::" + name)
    return matches[0] + "\n"

content = """/* GBA-to-C port translation bodies, selected verbatim.
 * E31/E42 already supply layer selection and world-to-tile coordinates. */
#include "area.h"
#include "asm.h"
#include "collision.h"
#include "entity.h"
#include "map.h"
#include "player.h"
#include "room.h"
#include "physics.h"
#include "structures.h"
#include "functions.h"
"""
for name in ("GetMapLayerForLayer", "NormalizeTilePos", "WorldToTilePos"):
    content += select("port/port_linked_stubs.c", name)
for name in ("GetTileTypeRelativeToEntity", "GetActTileAtTilePos",
             "GetActTileAtEntity", "GetActTileRelativeToEntity", "GetTileIndex"):
    content += select("port/port_linked_stubs.c", name)
content += "extern const KeyValuePair gMapActTileToSurfaceType[];\n"
for name in ("FindEntryForKeyInternal", "FindValueForKey", "CheckNEastTile", "PlayerCheckNEastTile"):
    content += select("port/port_gameplay_stubs.c", name)
for name in ("sub_0806F730",):
    content += select("src/physics.c", name)
content += select("src/scroll.c", "GetLayerByIndex")
(out / "dsi_original_tile_queries_e50.c").write_text(content)
