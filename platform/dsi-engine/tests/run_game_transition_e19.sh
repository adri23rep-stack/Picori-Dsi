#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
ROM=${1:-}
if [ ! -f "$ROM" ]; then
  echo 'Usage: sh platform/dsi-engine/tests/run_game_transition_e19.sh /path/to/baserom_eu.gba' >&2
  exit 2
fi
F='-std=gnu11 -O2 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
if [ "${DSI_E19_SANITIZE:-0}" = 1 ]; then
  SAN="-fsanitize=address,undefined -fno-omit-frame-pointer"
  LDSAN='-fsanitize=address,undefined'
else
  LDSAN=''
  SAN=''
fi
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
for MOD in title main common fileselect objectDefinitions game save; do
 cc $F -w $INC -include "$R/include/region.h" -c "$R/src/$MOD.c" -o "$TMP/$MOD.o"
done
for MOD in dsi_engine_state dsi_game_transition dsi_title_runner dsi_file_select_probe dsi_file_select_palette dsi_title_object_gfx dsi_title_entities dsi_title_trace dsi_title_sprites dsi_title_assets dsi_gba_bus dsi_rom_refs dsi_gba_renderer dsi_title_composite dsi_title_live; do
 cc $F -Wall -Wextra $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/$MOD.c" -o "$TMP/$MOD.o"
done
cc $F $SAN -Wall -Wextra $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_game_bootstrap.c" -o "$TMP/dsi_game_bootstrap.o"
# ORIGINAL FinalizeSave comes from unmodified playerUtils.c. Retain only
# that function using link-time garbage collection so unrelated 400+ player
# functions neither override test traps nor pull their dependencies in.
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/playerUtils.c" -o "$TMP/player_full.o"
ld -r --gc-sections -u FinalizeSave "$TMP/player_full.o" -o "$TMP/player_finalize.o"
cc $F $SAN -w $INC -include "$R/include/region.h" -c "$S/e19_game_test_shims.c" -o "$TMP/e19_shims.o"
cc $F $SAN -w $INC -c "$S/e19_unported_traps.c" -o "$TMP/e19_unported_traps.o"
cc $F -Wall -Wextra $INC -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/rom.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/object/titleScreenObject.c" -o "$TMP/titleScreenObject.o"
cc $F $SAN -w $INC -include "$R/include/region.h" -c "$S/engine_e19_game_transition_test.c" -o "$TMP/test.o"
cc $LDSAN -Wl,--gc-sections -Wl,--wrap=DispReset -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=MemClear -Wl,--wrap=zMallocInit $(find "$TMP" -maxdepth 1 -name "*.o" ! -name "player_full.o" -print) -o "$TMP/test"
"$TMP/test" "$ROM"
