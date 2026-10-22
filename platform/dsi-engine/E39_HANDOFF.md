# E39 — original ARCHWAY (0x4F) and correct player world origin

E38 melonDS screenshot showed:
- Verified original EU room 0x22/0x15, room 240x160, two entities (Link+obj), 3 managers.
- E33 `Bed: BLOCKED E:3` and `Bed f:FF x:-1`.
- E35 Link `x:144 y:56 rom:2`, sprite rejected as out-of-bounds.

**Actual root causes confirmed from upstream source:**
1. `include/object.h`: enum ARCHWAY=0x4F, BED_COVER=0x5B.
   Previous E33-E38 wrongly assumed 0x4F was a bed cover. It was the
   *genuine archway* object all along; ROM fixture and gObjectDefinition_4F
   match ARCHWAY. **Do not alter ROM id to BedCover.** The original
   `src/object/archway.c::Archway` is now compiled directly, action0
   dispatched once to the already-spawned native entity, and its original
   type2 animation frame is used.
2. Original EU room header for 0x22/0x15: map_x 0x510, map_y 0x1B0.
   `gRoomTransition.player_status.start_pos_x/y` are room-LOCAL offsets
   (proved by original `src/gameUtils.c::sub_08053250` which subtracts
   room origin before saving). Existing dummy slot started at 0x90,0x38.
   E29 original InitializePlayer previously copied those directly into
   world coordinates, placing Link 1000+ pixels outside the actual room.
   E39 verifies the real original InitRoom header and transforms the
   local transition status to (0x5A0,0x1E8) *before calling* original
   InitializePlayer; no change to save slot or fake actor.
3. Previous E34 `BedCover` rendering path disabled at runtime (source
   remains for regression, not called). Real original object graphics
   use the original `gObjectDefinition_4F` sprite/gfx metadata, and
   original frameIndex=type2 from Archway.

E39 host tests check original object enum mapping, action0->action1,
real collision layer/sprite initialization and strict relative->world
conversion, including bad-header and double-bind rejection. Full
regression and native NDS build on GitHub Actions. Still NOT playable:
original Link update/wakeup animation, interior collision, managers.
Screenshot validation needed; no ROM images bundled.

## E39 verified CI / artifact (2026-10-22)

- Passing code commit: `cf87ef4c10b5f02728e74769689471d0fd999986`
- Full CI: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39743991922
- E30–E39 host tests incl. corrected object ID and saved-local-to-world coordinate transform: PASS.
- Original src/object/archway.c compiled/linked ARM9, native symbols and NDS packaging: PASS.
- NDS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39743991922/artifacts/12410333242
- ARM9 build logs and symbol evidence: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39743991922/artifacts/12410193550

**MelonDS visual pending**. Inspect top room for sprite of original Link
(start local 144,56) and Archway (ROM ID 0x4F). Console should no longer
show missing "Bed" object; it should show `Arch: ACTIVE`, and room-local
Link drawing should return a ROM sprite reason code of zero. Do not infer
that an NDS CI success proves actual on-screen sprite pixels or playability.
