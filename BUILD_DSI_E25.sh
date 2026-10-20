#!/bin/sh
set -eu
HERE=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
cd "$HERE"
echo '[1/2] Host renderer tests (no ROM needed)'
sh platform/dsi-engine/tests/run_room_preview_e25.sh
echo '[2/2] Build ARM9 .nds (devkitPro required)'
: "${DEVKITPRO:?DEVKITPRO not set; install devkitPro/devkitARM and libnds}"
: "${DEVKITARM:=$DEVKITPRO/devkitARM}"
export DEVKITARM
make -C platform/dsi-engine clean
make -C platform/dsi-engine
printf 'NDS output: %s\n' "$HERE/platform/dsi-engine/tmc-dsi-engine-e25.nds"
