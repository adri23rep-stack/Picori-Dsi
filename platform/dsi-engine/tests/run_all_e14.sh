#!/bin/sh
# Host regression suite. Requires cc/GCC, original Europe ROM as argument.
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROM=${1:-}
if [ ! -f "$ROM" ]; then
  echo 'Usage: sh platform/dsi-engine/tests/run_all_e14.sh /path/to/baserom_eu.gba' >&2
  exit 2
fi
for test in run_host_test.sh run_title_e5.sh run_title_e6.sh \
            run_renderer_e7.sh run_title_e8.sh run_title_e9.sh \
            run_title_e10.sh run_title_e11.sh run_title_e12.sh \
            run_title_e13.sh run_file_select_e14.sh; do
  echo "== $test =="
  sh "$S/$test" "$ROM"
done
printf '%s\n' 'PASS: E3–E14 normal host regressions'
