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
