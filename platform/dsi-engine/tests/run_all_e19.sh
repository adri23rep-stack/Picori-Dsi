#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROM=${1:-}
if [ ! -f "$ROM" ]; then
  echo 'Usage: sh platform/dsi-engine/tests/run_all_e19.sh path/to/EU_ROM' >&2
  exit 2
fi
sh "$S/run_all_e17.sh" "$ROM"
sh "$S/run_game_bootstrap_e18.sh" "$ROM"
sh "$S/run_game_transition_e19.sh" "$ROM"
DSI_E19_SANITIZE=1 sh "$S/run_game_transition_e19.sh" "$ROM"
echo 'PASS E3-E19 including E18 bootstrap and E19 host transition/init (sanitized harness)'
