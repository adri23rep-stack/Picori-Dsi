#!/bin/sh
set -eu
ROOT=$(CDPATH='' cd -- "$(dirname "$0")/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_tile_queries_e50.py" "$ROOT" "$TMP"
python3 - "$ROOT" "$TMP/dsi_original_tile_queries_e50.c" <<'PY'
from pathlib import Path
import re,sys
root=Path(sys.argv[1]); output=Path(sys.argv[2]).read_text()
sources={
 "port/port_linked_stubs.c":(
  "GetMapLayerForLayer","NormalizeTilePos","WorldToTilePos",
  "GetTileTypeRelativeToEntity","GetActTileAtTilePos",
  "GetActTileAtEntity","GetActTileRelativeToEntity","GetTileIndex"),
 "port/port_gameplay_stubs.c":(
  "FindEntryForKeyInternal","FindValueForKey",
  "CheckNEastTile","PlayerCheckNEastTile"),
 "src/physics.c":("sub_0806F730",),
 "src/scroll.c":("GetLayerByIndex",)
}
for filename,names in sources.items():
 s=(root/filename).read_text()
 for n in names:
  p=r'^((?:static inline |static )?(?:MapLayer\*|u32|u16|s32|bool32|void) +[^\n;{}]*\b'+re.escape(n)+r'\([^\n;]*\) \{\n.*?^})'
  matches=re.findall(p,s,re.M|re.S)
  assert len(matches)==1 and matches[0] in output,(filename,n,len(matches))
PY
cc -std=gnu11 -O2 -g -w -ffunction-sections -fdata-sections \
 -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E41_COLLISION \
 -DNON_MATCHING -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
 -I"$ROOT/platform/dsi/source" -I"$ROOT/platform/dsi-engine/source" \
 -include "$ROOT/include/region.h" \
 -c "$TMP/dsi_original_tile_queries_e50.c" -o "$TMP/tiles.o"
for n in GetTileTypeRelativeToEntity GetActTileAtTilePos GetActTileAtEntity GetActTileRelativeToEntity FindValueForKey CheckNEastTile PlayerCheckNEastTile GetTileIndex GetLayerByIndex sub_0806F730; do
 nm --defined-only "$TMP/tiles.o" | grep -Eq " [Tt] $n$"
done
echo "E50/E52 original tile, layer, direction functions compiled from source"
