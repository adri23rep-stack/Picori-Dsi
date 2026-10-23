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
for filename, names in (
 ("port/port_linked_stubs.c",("GetTileTypeRelativeToEntity","GetActTileAtTilePos","GetActTileAtEntity","GetActTileRelativeToEntity")),
 ("port/port_gameplay_stubs.c",("FindEntryForKeyInternal","FindValueForKey"))):
 s=(root/filename).read_text()
 for n in names:
  m=re.search(r'^([^\n;{}]*\b'+re.escape(n)+r'\([^\n;]*\) \{\n.*?^})',s,re.M|re.S)
  assert m is not None and m.group(1) in output,n
PY
cc -std=gnu11 -O2 -g -w -ffunction-sections -fdata-sections \
   -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_E41_COLLISION \
   -DNON_MATCHING -DEU -DENGLISH -DREVISION=0 \
   -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
   -I"$ROOT/platform/dsi/source" -I"$ROOT/platform/dsi-engine/source" \
   -include "$ROOT/include/region.h" \
   -c "$TMP/dsi_original_tile_queries_e50.c" -o "$TMP/tiles.o"
for n in GetTileTypeRelativeToEntity GetActTileAtTilePos GetActTileAtEntity GetActTileRelativeToEntity FindValueForKey; do
 nm --defined-only "$TMP/tiles.o" | grep -Eq " [Tt] ${n}$"
done
echo "E50 original tile queries compiled and source matched"
