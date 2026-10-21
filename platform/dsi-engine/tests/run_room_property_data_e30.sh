#!/bin/sh
# E30.2: host-only fake ROM table/bytes. Tests data compatibility, not game.
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH='' cd -- "$HERE/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FLAGS='-std=gnu11 -O2 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$ROOT -I$ROOT/include -I$ROOT/port -I$ROOT/build/EU -I$ROOT/platform/dsi/source -I$ROOT/platform/dsi-engine/source"
cc $FLAGS $INC -include "$ROOT/include/region.h" \
 "$ROOT/platform/dsi-engine/source/dsi_room_property_data_e30.c" \
 "$HERE/engine_e30_room_property_data_test.c" -Wl,--gc-sections -o "$TMP/test"
"$TMP/test"
