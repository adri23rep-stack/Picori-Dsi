#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo 'Usage: run_all_e16.sh /path/to/owned_european_rom.gba' >&2; exit 2; }
sh "$S/run_all_e15.sh" "$ROM"
sh "$S/run_file_select_e16.sh" "$ROM"
sh "$S/run_bus_e16.sh" "$ROM"
echo 'PASS E3-E16 normal regressions'
