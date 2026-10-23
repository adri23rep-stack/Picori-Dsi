#!/bin/sh
set -eu
ROOT=$(CDPATH='' cd -- "$(dirname "$0")/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_extra_sprite_e47.py" "$ROOT" "$TMP"
python3 - "$ROOT/src/physics.c" "$TMP/dsi_original_extra_sprite_e47.c" <<'PY'
from pathlib import Path
import sys
source, generated = (Path(p).read_text() for p in sys.argv[1:])
for name in ("void sub_0806FD8C() {", "bool32 sub_0806FDA0(Entity* this) {"):
    start = source.index(name)
    end = source.index("\n}\n", start) + 3
    assert source[start:end] in generated, name
PY
cc -std=gnu11 -O1 -g -w -ffunction-sections -fdata-sections \
  -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI \
  -DNON_MATCHING -DEU -DENGLISH -DREVISION=0 \
  -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
  -I"$ROOT/platform/dsi/source" -I"$ROOT/platform/dsi-engine/source" \
  -include "$ROOT/include/region.h" \
  -c "$TMP/dsi_original_extra_sprite_e47.c" -o "$TMP/extra.o"
for symbol in sub_0806FD8C sub_0806FDA0 gUnk_020000C0 gPlayerClones; do
  nm --defined-only "$TMP/extra.o" | grep -Eq " [A-Za-z] ${symbol}$"
done
echo "E47 original physics.c extra-sprite functions/data host compile OK"
