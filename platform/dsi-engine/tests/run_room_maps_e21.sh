#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo 'Usage: run_room_maps_e21.sh /path/to/EU_ROM.gba' >&2; exit 2; }
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
CFLAGS="-std=gnu11 -O1 -g -Wall -Wextra -fno-omit-frame-pointer -I$R -I$R/include -I$R/port -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
if [ "${DSI_E21_SANITIZE:-0}" = 1 ]; then CFLAGS="$CFLAGS -fsanitize=address,undefined"; fi
cc $CFLAGS "$S/engine_e21_room_maps_test.c" \
  "$R/platform/dsi-engine/source/dsi_room_maps.c" \
  "$R/platform/dsi-engine/source/dsi_room_probe.c" \
  "$R/platform/dsi-engine/source/dsi_title_assets.c" \
  "$R/platform/dsi-engine/source/dsi_rom_refs.c" \
  "$R/platform/dsi-engine/source/dsi_gba_bus.c" \
  "$R/platform/dsi/source/tmc_rom_stream.c" -o "$TMP/room_maps_test"
"$TMP/room_maps_test" "$ROM"
