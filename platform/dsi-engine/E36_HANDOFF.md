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
