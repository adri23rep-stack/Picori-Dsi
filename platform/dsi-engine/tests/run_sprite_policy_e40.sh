#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
cc -std=gnu11 -g -O1 -ffunction-sections -fdata-sections \
 ${E40_SANITIZE:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
 -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
 -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/platform/dsi-engine/source" \
 -include "$ROOT/include/region.h" \
 "$ROOT/platform/dsi-engine/source/dsi_room_sprite_policy_e40.c" \
 "$HERE/engine_e40_sprite_policy_test.c" \
 -Wl,--gc-sections -o "$TMP/test"
ASAN_OPTIONS=detect_leaks=0 "$TMP/test"
# Guard the exact runtime ordering that passed visually: original
# room background -> authentic Archway action -> quarantined compositor
# -> original Link ROM sprite; never swap real player position with demo.
python3 - "$ROOT" <<'PY'
from pathlib import Path
import sys
root=Path(sys.argv[1])
s=(root/'platform/dsi-engine/source/boot_dsi.c').read_text()
must=['DsiRoomArchwayE39_Tick();','DsiRoomSpritePolicyE40_AllowPreview(',
      'DsiRoomArchwayGfxE39_Draw(','DsiRoomLinkE35_Draw(']
positions=[s.index(k) for k in must]
assert positions==sorted(positions),positions
assert 'DsiRoomObjectE33_Tick();' not in s
assert 'DsiRoomBedGfxE34_Draw(' not in s
g=(root/'platform/dsi-engine/source/dsi_room_archway_gfx_e39.c').read_text()
assert '(record&0x00fffffcu)' in g
assert '(record&0x00ffffffu)' not in g
print('E40 source-linked runtime order and original GFX packed flags PASS')
PY
