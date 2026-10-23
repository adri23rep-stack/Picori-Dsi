# E44 — Original Player ARM9 compilation checkpoint

Parent: `e41-original-interior-collision` at
`df3f85a0d4ddc0d562fc9f95d8bb6f9cf40e4772`.

Goal: prove the **unaltered upstream player action source** can compile as
a DSi ARM9 object, before attempting to link/execute native Link actions.

## What this commit changes

- Adds `tools/compile_player_e44.sh`, compiling the real `src/player.c`
  against the native ARM9 headers, same macros and optimization as the engine.
- Enforces that `DoPlayerAction`, `PlayerInit`, `PlayerSleep` are
  defined in the resulting ARM9 object, and exposes undefined references.
- CI compares the E43 exact relocation audit with symbols in the current
  **real** engine ELF, warning that a symbol's presence is not a working service.
- Leaves all existing original movement, collision, entity and room paths
  untouched. The full player object is intentionally **not linked or called**.

## Next blocker (do not skip)

Use `e44-player-unresolved.txt` and `e44-player-link-gap.md` from the CI
build evidence. Bring `PlayerInit` dependencies from upstream source into
native services in small groups; connect original `PlayerSleep` only when
script context / ExecuteScript and animation work. Only then wire
`DoPlayerAction` into guarded native tick, validate action and controlMode
without forcing inputs, and implement native OAM/sprite animation transport.

The successful ARM9 object compilation is **not** a playable build.
Keep the original source; do not substitute a custom Link movement loop.
Never commit GBA ROM assets, generated outputs, or unlicensed game data.

To reproduce on a devkitPro machine:

```sh
export DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM
sh platform/dsi-engine/tools/compile_player_e44.sh "$PWD"
python3 platform/dsi-engine/tools/audit_player_e43.py \
  --elf platform/dsi-engine/tmc-dsi-engine-e29.elf \
  --nm arm-none-eabi-nm
```
