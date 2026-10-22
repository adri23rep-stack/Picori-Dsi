# E33 — Original room object action tick

Branch: `e33-original-object-dispatch` based on passing E32.

Now extracting **verbatim source** routines from:
- `src/object/bedCover.c`: all six BedCover action routines.
- `src/objectUtils.c`: real CreateObject/CreateObjectWithParent.
- `src/physics.c`: original CopyPosition/PositionRelative.

At initial room load (0x22/0x15 EU), the real bed object ID 0x4F
(type=1) gets its original BedCover action0 -> action1 dispatch and
sprite frame configuration. Subsequent VBlank frames may call action1,
obeying the genuine EntityDisabled priority gate. A strict validator
requires the native entity/manager pool and first-room task state.

The original script interpreter, other objects and Ezlo managers are
NOT dispatched yet. Scripted bed variants are rejected before call, and
a wrap guard aborts any accidental ExecuteScript call; no fake success.
Neither the player's original PlayerNormal update nor gameplay sprite
graphics are enabled. Full playability requires those integrations.

Tests: `E33_SANITIZE=1 sh platform/dsi-engine/tests/run_objects_e33.sh`.
Runs E30, E31 and E32 regressions; checks native ARM9 original symbols.
Testing on melonDS and physical DSi still required.

## E33 CI checkpoint

- Passing commit: `7d95da6ddcb3917f79bc0b7356ca59741ffceca7`
- Run: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39714191181
- All E30/E31/E32 regressions + sanitized E33 room object test: SUCCESS.
- ARM9 linker and original-symbol provenance: SUCCESS.
- Download E33 NDS artifact: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39714191181/artifacts/12398187631

No melonDS/hardware validation. No current gameplay motion.
