#!/bin/sh
set -eu
DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror \
  -I"$DIR/../source" -I"$DIR/../../dsi/source" \
  "$DIR/../source/dsi_entity_lists.c" \
  "$DIR/engine_e28_entity_lists_test.c" -o "$TMP/e28"
"$TMP/e28"
