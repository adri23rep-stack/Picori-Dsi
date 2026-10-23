#!/bin/sh
set -eu
ROOT=$(CDPATH='' cd -- "$(dirname "$0")/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_physics_e49.py" "$ROOT" "$TMP"
python3 - "$ROOT" "$TMP/dsi_original_physics_e49.c" <<'PY'
from pathlib import Path
import sys
root, path = Path(sys.argv[1]), Path(sys.argv[2])
generated = path.read_text()
for source_name, name in (
    ("src/physics.c", "s16 FixedDiv(s16 r0, s16 r1) {"),
    ("src/physics.c", "void LinearMoveUpdate(Entity* ent) {"),
    ("port/port_linked_stubs.c", "u32 GravityUpdate(Entity* entity, u32 gravity) {"),
):
    source = (root / source_name).read_text()
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
  -c "$TMP/dsi_original_physics_e49.c" -o "$TMP/physics.o"
for sym in FixedDiv LinearMoveUpdate GravityUpdate; do
  nm --defined-only "$TMP/physics.o" | grep -Eq " [Tt] ${sym}$"
done
echo "E49 original gravity and movement functions host compiled OK"
