#!/usr/bin/env python3
"""Select the original PlayerInit body verbatim; no rewritten movement/state logic."""
from pathlib import Path
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: generate_player_init_e45.py ROOT OUTDIR")
root, out = map(Path, sys.argv[1:])
source = (root / "src/player.c").read_text()
# Keep upstream includes, constants and forward declarations. The first
# function comes after these; do not parse nested C with a regex.
first_function = "\nbool32 CheckInitPauseMenu(void) {"
signature = "static void PlayerInit(PlayerEntity* this) {"
if source.count(first_function) != 1 or source.count(signature) != 1:
    raise SystemExit("Upstream player.c structure changed; inspect extraction")
prelude = source[:source.index(first_function)]
start = source.index(signature)
end = source.find("\n}\n", start)
if end < 0:
    raise SystemExit("Original PlayerInit closing brace missing")
body = source[start:end + 3]
required = ("LoadSwapGFX(super, 1, 2)", "InitAnimationForceUpdate(super, 2)",
            "CheckQueuedAction()", "PlayerSetNormalAndCollide()")
if not all(token in body for token in required):
    raise SystemExit("Unexpected upstream PlayerInit body")
# GBA source function remains byte-for-byte identical. Exposed entry remains
# unreachable in the linked ELF until real engine dependencies are ready.
output = ("/* E45: literal upstream PlayerInit, native ARM9 compilation only. */\n"
          + prelude + "\n" + body + "\n"
          + "void DsiPlayerE45_OriginalInit(PlayerEntity* player) {\n"
          + "    PlayerInit(player);\n}\n")
out.mkdir(parents=True, exist_ok=True)
(out / "dsi_player_init_e45.c").write_text(output)
