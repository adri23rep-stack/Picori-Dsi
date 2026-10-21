#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror \
  -I"$HERE/../source" -I"$HERE/../../dsi/source" \
  "$HERE/../source/dsi_room_properties_e30.c" \
  "$HERE/engine_e30_room_properties_test.c" -o "$TMP/test"
"$TMP/test"
