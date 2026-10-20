#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
cc -std=c11 -O2 -Wall -Wextra -Werror -Isource \
  source/tmc_rom_stream.c source/tmc_gfx_preview.c tests/test_gfx_preview.c \
  -o /tmp/tmc_dsi_gfx_preview_test
/tmp/tmc_dsi_gfx_preview_test
