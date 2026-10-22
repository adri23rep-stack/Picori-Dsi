# E36 — Original PlayerInit and PlayerNormal SOURCE COMPILED

Base: E35 native collision MapLayer and corrected source bed sprite.

`tools/generate_lifecycle_e31.py` now extracts the complete original,
UNEDITED function bodies from `src/player.c`: `PlayerInit` and
`PlayerNormal`. Both compile as actual host + devkitARM source (not mock
code). No movement algorithm or player controller is recreated.

**Critical limitation**: these functions are currently included in a
separate relocatable object but linker-GC'd from the final .nds: the native
DSi build still lacks the original animation loader, collision resolver,
queued-action handlers, item and script support. E36 emits
`e36-player-unresolved.txt` from the source TU to make the exact missing
dependency closure visible for the next stage. We never bypass the original
`DoPlayerAction` dispatcher to invoke incomplete actions.

All previous E30-E35 host regressions remain, E36 adds a source compilation
smoke test. User's own EU GBA ROM is required externally. Not playable yet.

## E36 proof / last known passing checkpoint

- Passing source commit: `fb59e51d16bce4c598dbb7b3a958ccd694be9888`
- CI run: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720548219
- All host regressions E30-E36: SUCCESS.
- ARM9 compile/link and `PlayerInit`/`PlayerNormal` source object: SUCCESS.
- NDS download: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720548219/artifacts/12400843331
- Build log + E36 player undefined-symbol closure:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720548219/artifacts/12400643852

**Immediate next tasks:** consume `e36-player-unresolved.txt`,
compile/link original animation manager and collision movement dependencies,
bind the original complete `DoPlayerAction` dispatcher rather than calling
individual action routines out of order, and render Link's original ROM
sprite in the room. Gate real gameplay on both original collision tables
and source player lifecycle. Do not call this playable until Link actually
moves with solid walls/room transitions under melonDS/DSi testing.
