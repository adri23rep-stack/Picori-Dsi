#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo 'Usage: run_game_native_e23.sh baserom_eu.gba' >&2; exit 2; }
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM
F='-std=gnu11 -O2 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_SAFE_GAME_BOOT -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
if [ "${DSI_E24_SANITIZE:-0}" = 1 ]; then SAN='-fsanitize=address,undefined -fno-omit-frame-pointer'; else SAN=''; fi
for MOD in title main common fileselect objectDefinitions game save playerUtils; do
  cc $F -w $INC -include "$R/include/region.h" -c "$R/src/$MOD.c" -o "$TMP/$MOD.o"
done
for MOD in dsi_engine_state dsi_game_transition dsi_game_native dsi_game_bootstrap dsi_title_runner dsi_file_select_probe dsi_file_select_palette dsi_title_object_gfx dsi_title_entities dsi_title_trace dsi_title_sprites dsi_title_assets dsi_gba_bus dsi_rom_refs dsi_gba_renderer dsi_title_composite dsi_title_live; do
  cc $F $(case $MOD in dsi_game_native|dsi_game_transition|dsi_game_bootstrap) printf %s "$SAN" ;; esac) -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/$MOD.c" -o "$TMP/$MOD.o"
done
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/object/titleScreenObject.c" -o "$TMP/titleScreenObject.o"
cc $F -w $INC -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/rom.o"
cc $F $SAN -w $INC -include "$R/include/region.h" -c "$S/engine_e24_room_prefix_test.c" -o "$TMP/test.o"
cc $SAN -Wl,--gc-sections -Wl,--wrap=DispReset -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=MemClear -Wl,--wrap=zMallocInit "$TMP"/*.o -o "$TMP/e24_test"
"$TMP/e24_test" "$ROM"
