#!/bin/sh
set -eu
S=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
R=$(CDPATH= cd -- "$S/../../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM
F='-std=gnu11 -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DTMC_DSI_SAFE_GAME_BOOT -DTMC_DSI_E26_ORIGINAL_MESSAGE -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
INC="-I$R -I$R/include -I$R/port -I$R/build/EU -I$R/platform/dsi/source -I$R/platform/dsi-engine/source"
cc $F -w $INC -include "$R/include/region.h" -c "$R/src/message.c" -o "$TMP/message.o"
cc $F -w $INC -include "$R/include/region.h" -c "$S/engine_e26_message_test.c" -o "$TMP/test.o"
cc -Wl,--gc-sections -Wl,--wrap=MessageInitialize "$TMP/message.o" "$TMP/test.o" -o "$TMP/test"
"$TMP/test"
