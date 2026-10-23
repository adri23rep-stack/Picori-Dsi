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
