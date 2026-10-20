#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT HUP INT TERM
F="-std=gnu11 -O1 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0"
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
# Original source files (untouched) compiled and linked into test binary.
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/common.c" -o "$T/common.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/main.c" -o "$T/main.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/flags.c" -o "$T/flags.o"
# shellcheck disable=SC2086
cc $F -Wall -Wextra $INC -c "$R/platform/dsi-engine/source/dsi_engine_state.c" -o "$T/state.o"
# shellcheck disable=SC2086
cc $F -Wall -Wextra $INC -c "$R/platform/dsi-engine/source/dsi_rom_refs.c" -o "$T/refs.o"
# shellcheck disable=SC2086
cc $F -Wall -Wextra $INC -c "$R/platform/dsi/source/tmc_rom_stream.c" -o "$T/rom.o"
# shellcheck disable=SC2086
cc $F -Wall -Wextra $INC -c "$R/platform/dsi-engine/source/dsi_gba_bus.c" -o "$T/bus.o"
# shellcheck disable=SC2086
cc $F -w $INC -include "$R/include/region.h" -c "$S/engine_e3_test.c" -o "$T/test.o"
cc -Wl,--gc-sections "$T/common.o" "$T/main.o" "$T/flags.o" "$T/state.o" "$T/refs.o" "$T/rom.o" "$T/bus.o" "$T/test.o" -o "$T/test"
"$T/test" "$@"
echo "PASS: src/main.c, src/common.c & src/flags.c compiled; original input, tasks and flags executed"
