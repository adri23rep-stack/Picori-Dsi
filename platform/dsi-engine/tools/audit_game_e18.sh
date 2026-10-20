#!/bin/sh
# Compile src/game.c unchanged, then describe its entry-state dependencies.
# Does not run any gameplay or link the PC game into the DSi executable.
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
F='-std=gnu11 -O2 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/game.c" -o "$TMP/game.o"
if ! command -v nm >/dev/null || ! command -v objdump >/dev/null; then
  echo 'Missing nm or objdump' >&2; exit 1
fi
COUNT=$(nm -u "$TMP/game.o" | wc -l | tr -d ' ')
echo "PASS E18: original src/game.c compiles unchanged on PC"
echo "Unresolved object-level externals (not unique subsystems): $COUNT"
for FN in GameTask_Transition GameTask_Init GameMain_InitRoom; do
  echo "== $FN direct references =="
  objdump -r "$TMP/game.o" | awk -v section=".text.$FN" '
    /^RELOCATION RECORDS FOR \[/ { active = index($0,"[" section "]") > 0; next }
    active && /^[0-9a-f]+/ { print $3 }
  ' | sed -E 's/[+-]0x[0-9a-f]+$//' | sort -u
  echo
 done
