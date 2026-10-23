# E45 — original PlayerInit: native DSi build, guarded

Parent E44: `74a94c44048a002da5f69aa3a42f5c3cc045f2cf`. Original source `src/player.c`.

The generated `dsi_player_init_e45.c` contains the **unchanged**
`static void PlayerInit(PlayerEntity* this)` body and original include/macro
prelude. Only a new external wrapper is added so future native code may call it
after validating animation/GFX/items state. No replacement C implementation.

Host checks compile the real body and verify literal source matching.
ARM9's engine Makefile also compiles it into its object, but the engine link
**must discard the unused wrapper**. CI checks it is defined in the ARM object
and absent from the final ELF. This is deliberate: the room still spawns Link
in the sleeping state and scripts/animation transport remain incomplete.
Running PlayerInit now would be unsafe; there are no fake missing services.

**Next:** port and link genuine definitions for `CheckQueuedAction`,
`CreateItemEquippedAtSlot`, `DeleteClones`, `InitAnimationForceUpdate`,
`IsItemEquipped`, `LoadSwapGFX`, `PlayerSetNormalAndCollide`,
`PlayerSwimming`, `ResolvePlayerAnimation`, `UpdateFloorType`,
`UpdatePlayerSkills`, `sub_0806FDA0` before enabling this wrapper.
Most of these are already source functions, but `present` is not the same as
verified in-game behavior. Do not invent stubs, skip waking scripts, or force
controlMode; do not upload ROM or generated data.
