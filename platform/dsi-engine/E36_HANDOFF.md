# E36 — Native room boundary collision from unmodified original source

Base: E35 static original Link sprite (`e35-original-link-rom-sprite`).
The user-verified E34 build remains intact.

This stage executes the **actual** `CreateCollisionDataBorderAroundRoom`
in `src/playerUtils.c` (already compiled by ARM9) against the two
actual original `MapLayer` instances instantiated in E31. The function
marks bottom/top collision borders using the room dimensions and GBA
64x64 map stride. No duplicate collision rules or game movement.

The DSi adapter checks the real E31 room is loaded, ROM metadata equals
`gRoomControls`, and offsets fit the original 4096-cell arrays. After
the source call it independently verifies all 4 outer boundaries
on BOTH original layers. Host regression extracts the original function
body verbatim and exercises it under ASan/UBSan.

**CRITICAL: NOT PLAYABLE.** Interior collisionData for the room is not
yet derived from all original metatile types via `sub_0807BBE4`
and the actual GBA tile type->collision lookup tables. Do not enable
`CONTROL_ENABLED`, `PlayerNormal` or position changes until this is
complete, even though the ROM Link sprite is now visible in E35.
Original save, scripts and managers still blocked. Needs melonDS test.

## E36 passing CI / ARM9

- Passing code commit: `c8f2947304240880635d51503a0a67d311446942`
- Full workflow: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720564777
- E30-E36 host regressions incl. original collision routine with ASan/UBSan: PASS
- devkitARM9 NDS link + original-symbol checks: PASS
- NDS artifact: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720564777/artifacts/12401126218
- Build logs/symbol evidence: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39720564777/artifacts/12400933181

Still no visual test in melonDS for E35/E36, and NO player movement.
The collision marker only covers map outer boundaries, NOT interior walls.
