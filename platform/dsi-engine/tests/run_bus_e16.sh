#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo 'Usage: run_bus_e16.sh /path/to/owned_european_rom.gba' >&2; exit 2; }
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FLAGS="-std=gnu11 -O1 -g -Wall -Wextra -fno-omit-frame-pointer -I$R -I$R/include -I$R/port -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
if [ "${DSI_E16_SANITIZE:-0}" = 1 ]; then FLAGS="$FLAGS -fsanitize=address,undefined"; fi
cc $FLAGS "$S/engine_e16_bus_test.c" "$R/platform/dsi-engine/source/dsi_gba_bus.c" \
    "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/bus_test"
"$TMP/bus_test" "$ROM"
