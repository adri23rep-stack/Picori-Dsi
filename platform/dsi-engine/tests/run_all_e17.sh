#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROM=${1:-}
[ -f "$ROM" ] || { echo 'Provide a valid European ROM path' >&2; exit 2; }
sh "$S/run_all_e16.sh" "$ROM"
sh "$S/run_file_select_e17.sh" "$ROM"
echo 'PASS E3-E17 normal regressions'
