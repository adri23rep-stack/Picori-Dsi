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

## Verified CI on 2026-10-21 local

- Code commit: `41831aa0cec97b5da363c957bbd873f292ea19d6`
- Workflow: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39719584048
- Host E30-E35 regressions: SUCCESS, including sanitized Link clipping tests.
- Native ARM9 build and source-symbol provenance: SUCCESS.
- NDS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39719584048/artifacts/12400831679
- Build logs: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39719584048/artifacts/12400931368

No E35 melonDS screenshot yet. ROM original Link frame0 only. There is
no player movement/animation update. E34 is still preserved as working baseline.
