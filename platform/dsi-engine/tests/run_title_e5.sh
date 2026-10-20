#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
F='-std=gnu11 -O1 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
for MOD in title main common objectDefinitions; do
    # shellcheck disable=SC2086
    cc $F -w $INC -include "$R/include/region.h" -c "$R/src/$MOD.c" -o "$T/$MOD.o"
done
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_title_trace.c" -o "$T/trace.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_title_object_gfx.c" -o "$T/objectgfx.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_title_entities.c" -o "$T/entities.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/object/titleScreenObject.c" -o "$T/titleScreenObject.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/tests/title_e5_test.c" -o "$T/test.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_gba_bus.c" -o "$T/bus.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$T/rom.o"
# shellcheck disable=SC2086
cc $F -w $INC -c "$R/platform/dsi-engine/source/dsi_title_assets.c" -o "$T/assets.o"
# shellcheck disable=SC2086
cc $F -w $INC -c "$R/platform/dsi-engine/source/dsi_rom_refs.c" -o "$T/refs.o"
cc $F -w $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/dsi_title_sprites.c" -o "$T/sprites.o"
cc -Wl,--gc-sections -Wl,--wrap=DispReset -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=MemClear "$T/title.o" "$T/main.o" "$T/common.o" "$T/trace.o" "$T/assets.o" "$T/refs.o" "$T/bus.o" "$T/rom.o" "$T/sprites.o" "$T/entities.o" "$T/objectgfx.o" "$T/objectDefinitions.o" "$T/titleScreenObject.o" "$T/test.o" -o "$T/test"
"$T/test"
