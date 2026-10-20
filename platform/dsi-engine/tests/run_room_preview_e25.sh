#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
BIN=$(mktemp /tmp/tmc_e25_test.XXXXXXXX)
trap 'rm -f "$BIN"' EXIT HUP INT TERM
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror \
    -I"$HERE/../source" -I"$HERE/../../dsi/source" \
    "$HERE/../source/dsi_room_preview.c" \
    "$HERE/engine_e25_preview_test.c" -o "$BIN"
"$BIN"
