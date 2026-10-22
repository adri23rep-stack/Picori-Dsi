# E35 — Render actual Link entity using the original EU ROM sprite data

Based on user's visually-tested E34, protected in `e34-original-room-object-gfx`.

- Uses the existing, ROM-backed `platform/dsi/source/tmc_link_sprite.c`,
  whose EU offsets are traced to the project's ROM asset indices, and its
  real 50 Link metasprite frame records, plus tunic palette.
- This is NOT a custom drawing or a substitute gameplay engine. The Link
  placement reads the actual `gPlayerEntity.base` created by original
  `InitializePlayer()`. The original first room and bed remain untouched.
- Displays ROM frame 0, static, while `PlayerInit` and `DoPlayerAction` are
  not linked: animation is not simulated. No movement is fabricated.
- Host sanitized tests cover clipping/palette/transparent pixels, and CI
  checks that source-linked Link gfx + ROM decoder are in the native NDS.
- Does not upload or embed copyrighted ROM graphics in the NDS.
- Follow-on: port actual player action dispatch, collision, animation and
  transition. E35 still cannot honestly be called playable.
