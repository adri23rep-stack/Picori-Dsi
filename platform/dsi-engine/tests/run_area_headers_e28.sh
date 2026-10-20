#!/bin/sh
set -eu
S=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH='' cd -- "$S/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
${CC:-cc} -std=gnu11 -O2 -Wall -Wextra -ffunction-sections -fdata-sections \
  -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0 \
  -I"$R" -I"$R/include" -I"$R/port" -I"$R/build/EU" \
  -I"$R/platform/dsi/source" -I"$R/platform/dsi-engine/source" \
  -include "$R/include/region.h" \
  "$R/platform/dsi-engine/source/dsi_area_headers.c" \
  "$S/engine_e28_area_headers_test.c" -o "$TMP/test"
"$TMP/test"
