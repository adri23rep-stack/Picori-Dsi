#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
ROM=${1:-}
if [ ! -f "$ROM" ]; then
    echo "E6: pass EU baserom as first argument: sh platform/dsi-engine/tests/run_title_e6.sh /path/to/baserom_eu.gba" >&2
    exit 2
fi
python3 - "$T/synthetic.gba" <<'PY'
import sys
b=bytearray(1024)
b[0xAC:0xB0]=b'BZMP'
# LZ77 header, output 12 bytes, four literals and an 8-byte backreference distance 4.
b[0x300:0x300+11]=bytes([0x10, 12, 0, 0, 0x08, 65, 66, 67, 68, 0x50, 0x03])
open(sys.argv[1],'wb').write(b)
PY
F='-std=gnu11 -O1 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
for MOD in title main common objectDefinitions; do
    # shellcheck disable=SC2086
    cc $F -w $INC -include "$R/include/region.h" -c "$R/src/$MOD.c" -o "$T/$MOD.o"
done
for MOD in dsi_title_object_gfx dsi_title_entities dsi_title_trace dsi_title_sprites dsi_title_assets dsi_gba_bus dsi_rom_refs; do
    # shellcheck disable=SC2086
    cc $F -Wall -Wextra $INC -include "$R/include/region.h" -c "$R/platform/dsi-engine/source/$MOD.c" -o "$T/$MOD.o"
done
# shellcheck disable=SC2086
cc $F -Wall -Wextra $INC -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$T/rom.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/object/titleScreenObject.c" -o "$T/titleScreenObject.o"
cc $F -w $INC -include "$R/include/region.h" -c "$S/engine_e6_test.c" -o "$T/test.o"
cc -Wl,--gc-sections -Wl,--wrap=DispReset -Wl,--wrap=LoadGfxGroup -Wl,--wrap=LoadPaletteGroup -Wl,--wrap=MemClear "$T"/*.o -o "$T/test"
"$T/test" "$ROM" "$T/synthetic.gba"
