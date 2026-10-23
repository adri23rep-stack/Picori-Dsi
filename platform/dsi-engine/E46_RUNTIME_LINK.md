# E46 experimental runtime-link lab

From E45 (preserved at `87c4a4f31b7ef40173d22ea7b10afb94e2205afb`).
For the first time the native ARM9 ELF is told to retain
`DsiPlayerE45_OriginalInit`. This immediately exercises the unresolved
original upstream dependencies. This is an intentionally aggressive
experimental branch: a red linker job is a dependency inventory, **not**
a playable build or a replacement engine.

The original code, room graphics and collision remain unchanged.
No PlayerInit call is added to runtime until a genuine supported entry
and original animation/script chain exist. Never add empty success stubs
or force CONTROL_ENABLED to make a screenshot move.

See CI output for original missing symbols and resolve them using their
genuine functions (mostly in `src/playerUtils.c`, animation/gfx).

## E46.1 — native originals, broad compilation

The initial forced-link report found missing original `DoPlayerAction`,
`gItemDefinitions`, `gMapActTileToSurfaceType`, the complete surface callback
table, `LoadSwapGFX`, `sub_0806FDA0`, `InitAnimationForceUpdate`,
`IsItemEquipped`, and others.

The Makefile now compiles genuine upstream `src/player.c`,
`src/itemDefinitions.c`, `src/itemUtils.c`, `src/itemMetaData.c`,
`src/data/mapActTileToSurfaceType.c` into the ARM9 engine. Crucially,
`CheckQueuedAction` can now resolve the real player dispatcher rather than
a dummy. Expect the original complete action table to surface further
dependencies. Existing room boot still does not call that dispatch.

This branch is allowed to remain red until the entire dependency closure
links; original E45 and E41 remain stable.

## E47: real sprite slot memory replaces fake reset

New generator selects `sub_0806FD8C` and `sub_0806FDA0`
**verbatim** from `src/physics.c`. The former clears the original
extra-sprite bank; the latter reserves a slot for an original Entity,
used directly by PlayerInit. The native DSi side-table and
`gPlayerClones[3]` are actual relocated GBA globals, not substituted
movement or entity AI. The previous `sub_0806FD8C` counter stub
was removed so the reset is real.

Note: compiler/link success still does NOT establish that the final native
SpritePtr/animation resource loader is ready. E47 does not execute wake-up.

## E48 — upstream GFX slot manager compilation

The original `src/vram.c` now joins native ARM9 build directly, including
`LoadSwapGFX`, slot reservations/references and their genuine management
routines. This is the GBA→DSi port's actual original resource manager,
not a recreated player animation or fake LoadSwapGFX handler.

**Hardware caveat:** GBA VRAM ownership and the original animation pointer
table are NOT yet translated to DSi VRAM/OAM. Full runtime PlayerInit is
not enabled and is still intentionally unresolved. Do not call hardware
transfer paths without mapping native VRAM and validating ROM sources.
