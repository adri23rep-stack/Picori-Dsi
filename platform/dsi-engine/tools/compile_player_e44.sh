#!/bin/sh
# E44: compile the upstream src/player.c for DSi ARM9 *without linking it*.
# Only record real definitions/undefined symbols. This does not enable play.
set -eu
ROOT=$(CDPATH='' cd -- "$(dirname -- "$0")/../../.." && pwd)
: "${DEVKITPRO:?devkitPro is required}"
: "${DEVKITARM:?devkitARM is required}"
OUT=${1:-"$ROOT/platform/dsi-engine"}
mkdir -p "$OUT"
CC="$DEVKITARM/bin/arm-none-eabi-gcc"
NM="$DEVKITARM/bin/arm-none-eabi-nm"
test -x "$CC" && test -x "$NM"
"$CC" -std=gnu11 -O2 -g -w \
    -march=armv5te -mtune=arm946e-s -mthumb \
    -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv \
    -DPC_PORT -DTMC_DSI -DTMC_DSI_SAFE_GAME_BOOT \
    -DTMC_DSI_E26_ORIGINAL_MESSAGE -DTMC_DSI_E28_ORIGINAL_INITROOM \
    -DTMC_DSI_E29_ENTITY_CORE -DTMC_DSI_E30_PREP -DTMC_DSI_E31_LIFECYCLE \
    -DTMC_DSI_E41_COLLISION -DNON_MATCHING -DUSE_HDMA -DEU \
    -DENGLISH -DREVISION=0 -DARM9 \
    -include "$ROOT/include/region.h" \
    -I"$ROOT/platform/dsi-engine/source" -I"$ROOT" -I"$ROOT/include" \
    -I"$ROOT/port" -I"$ROOT/build/EU" \
    -I"$ROOT/platform/dsi/source" \
    -I"$DEVKITPRO/libnds/include" -I"$DEVKITPRO/portlibs/nds/include" \
    -c "$ROOT/src/player.c" -o "$OUT/e44-player-original.o"
"$NM" "$OUT/e44-player-original.o" > "$OUT/e44-player-original-symbols.txt"
"$NM" -u "$OUT/e44-player-original.o" > "$OUT/e44-player-unresolved.txt"
for symbol in DoPlayerAction PlayerInit PlayerSleep; do
    if ! grep -Eq " [Tt] ${symbol}(\\.[[:alnum:]_.]+)?$" "$OUT/e44-player-original-symbols.txt"; then
        echo "E44: missing original symbol $symbol in ARM9 object" >&2
        exit 1
    fi
done
test -s "$OUT/e44-player-unresolved.txt"
echo "E44: original src/player.c ARM9 object compiled; dependencies still unresolved."
echo "E44: gameplay dispatch remains OFF; no original player behavior was replaced."
