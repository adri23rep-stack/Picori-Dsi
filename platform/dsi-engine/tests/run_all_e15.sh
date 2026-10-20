#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROM=${1:-}
if [ ! -f "$ROM" ]; then
  echo 'Usage: run_all_e15.sh /path/to/owned_EU_ROM.gba' >&2
  exit 2
fi
sh "$S/run_all_e14.sh" "$ROM"
sh "$S/run_file_select_e15.sh" "$ROM"
echo 'PASS E3-E15 normal regressions'
