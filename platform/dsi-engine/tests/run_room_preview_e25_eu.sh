#!/bin/sh
set -eu
S=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH='' cd -- "$S/../../.." && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo "Usage: $0 /path/to/baserom_eu.gba [preview.ppm]" >&2; exit 2; }
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
CFLAGS="-std=gnu11 -O1 -g -Wall -Wextra -I$R -I$R/include -I$R/port -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
cc $CFLAGS "$S/engine_e25_rom_preview_test.c" \
    "$R/platform/dsi-engine/source/dsi_room_preview.c" \
    "$R/platform/dsi-engine/source/dsi_room_tiles.c" \
    "$R/platform/dsi-engine/source/dsi_room_maps.c" \
    "$R/platform/dsi-engine/source/dsi_room_probe.c" \
    "$R/platform/dsi-engine/source/dsi_title_assets.c" \
    "$R/platform/dsi-engine/source/dsi_rom_refs.c" \
    "$R/platform/dsi-engine/source/dsi_gba_bus.c" \
    "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/room_preview"
"$TMP/room_preview" "$ROM" "${2:-$TMP/preview.ppm}"
if [ $# -lt 2 ]; then echo "Note: preview output was temporary; pass a .ppm path to keep it."; fi
