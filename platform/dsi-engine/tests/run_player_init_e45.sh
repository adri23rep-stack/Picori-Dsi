#!/bin/sh
# Host compile-only regression: original player init extraction / ABI.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
python3 "$ROOT/platform/dsi-engine/tools/generate_player_init_e45.py" "$ROOT" "$TMP"
python3 - "$ROOT/src/player.c" "$TMP/dsi_player_init_e45.c" <<'PY'
import pathlib, sys
original = pathlib.Path(sys.argv[1]).read_text()
generated = pathlib.Path(sys.argv[2]).read_text()
sig = "static void PlayerInit(PlayerEntity* this) {"
start = original.index(sig)
end = original.index("\n}\n", start) + 3
assert original[start:end] in generated, "Original PlayerInit body changed"
assert "void DsiPlayerE45_OriginalInit(PlayerEntity* player)" in generated
PY
cc -std=gnu11 -O2 -g -w -ffunction-sections -fdata-sections \
   -fno-strict-aliasing -fwrapv \
   -DPC_PORT -DTMC_DSI -DTMC_DSI_E41_COLLISION -DNON_MATCHING \
   -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
   -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" \
   -I"$ROOT/platform/dsi/source" -I"$ROOT/platform/dsi-engine/source" \
   -include "$ROOT/include/region.h" \
   -c "$TMP/dsi_player_init_e45.c" -o "$TMP/player.o"
nm "$TMP/player.o" | grep -Eq ' [Tt] DsiPlayerE45_OriginalInit$'
echo "E45: original PlayerInit extracted verbatim and host compiled; not executed."
