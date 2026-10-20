#!/bin/sh
# Builds ORIGINAL src/main.c and src/title.c for dependency inspection.
# Does not claim a runnable DSi title screen. Requires host C compiler and nm.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/../../.." && pwd)
CC=${CC:-cc}
NM=${NM:-nm}
command -v "$CC" >/dev/null 2>&1 || { echo "ERROR: host compiler '$CC' not found" >&2; exit 1; }
command -v "$NM" >/dev/null 2>&1 || { echo "ERROR: nm tool '$NM' not found" >&2; exit 1; }
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FLAGS='-std=gnu11 -O1 -g -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -DPC_PORT -DTMC_DSI -DNON_MATCHING -DUSE_HDMA -DEU -DENGLISH -DREVISION=0'
for MOD in main title; do
    # shellcheck disable=SC2086
    "$CC" $FLAGS -w -I"$ROOT" -I"$ROOT/include" -I"$ROOT/port" -I"$ROOT/build/EU" \
       -include "$ROOT/include/region.h" -c "$ROOT/src/$MOD.c" -o "$TMP/$MOD.o"
    echo "PASS compile original src/$MOD.c (host)"
    echo "External symbols in src/$MOD.c:"
    "$NM" -u "$TMP/$MOD.o" | sed 's/^[[:space:]]*U /  /' | sort
    echo "----"
done
echo "NOTE: compilation only. No AgbMain(), TitleTask(), graphics or sound executed."
