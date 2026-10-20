#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
ROM=${1:-}
if [ ! -f "$ROM" ]; then
  echo 'Usage: sh platform/dsi-engine/tests/run_title_e8.sh /path/to/baserom_eu.gba [preview.ppm]' >&2
  exit 2
fi
F='-std=gnu11 -O2 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
for MOD in title main common objectDefinitions; do
 cc $F -w $INC -include "$R/include/region.h" -c "$R/src/$MOD.c" -o "$TMP/$MOD.o"
done
for MOD in dsi_title_object_gfx dsi_title_entities dsi_title_trace dsi_title_sprites dsi_title_assets dsi_gba_bus dsi_rom_refs dsi_gba_renderer dsi_title_composite; do
 cc $F -Wall -Wextra $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/$MOD.c" -o "$TMP/$MOD.o"
done
cc $F -Wall -Wextra $INC -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/rom.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/object/titleScreenObject.c" -o "$TMP/titleScreenObject.o"
cc $F -w $INC -include "$R/include/region.h" -c "$S/engine_e8_title_test.c" -o "$TMP/test.o"
cc -Wl,--gc-sections -Wl,--wrap=DispReset -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=MemClear "$TMP"/*.o -o "$TMP/test"
"$TMP/test" "$ROM" "${2:-}"
