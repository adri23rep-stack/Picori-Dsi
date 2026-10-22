#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 - "$ROOT" "$TMP" <<'PY'
from pathlib import Path
import sys,re
root,tmp=map(Path,sys.argv[1:3])
source=(root/'src/playerUtils.c').read_text()
m=re.search(r'^void CreateCollisionDataBorderAroundRoom\(void\) \{\n.*?^\}',source,re.M|re.S)
if not m: raise SystemExit('Original border source changed: refuse synthetic replacement')
(tmp/'original_border.c').write_text('#include "room.h"\n#include "map.h"\n'+m.group()+'\n')
PY
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
 ${E36_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" \
 -include "$ROOT/include/region.h" \
 "$TMP/original_border.c" \
 "$ROOT/platform/dsi-engine/source/dsi_room_collision_e36.c" \
 "$HERE/engine_e36_border_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
