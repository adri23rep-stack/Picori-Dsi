#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
BIN="${TMPDIR:-/tmp}/tmc_dsi_v4_link_test"
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -Isource tests/test_link_sprite.c \
  source/tmc_link_sprite.c source/tmc_rom_stream.c -o "$BIN"
if [ "$#" -lt 1 ]; then
  echo "Usage: sh tests/run_link_tests.sh /path/to/your/baserom_eu.gba" >&2
  exit 2
fi
"$BIN" "$1"
