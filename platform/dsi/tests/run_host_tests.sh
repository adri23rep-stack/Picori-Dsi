#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
cc -std=c11 -Wall -Wextra -Werror -O2 -Isource \
  tests/test_rom_stream.c source/tmc_rom_stream.c source/tmc_input.c -o tests/tmc_test
./tests/tmc_test
rm -f tests/tmc_test
