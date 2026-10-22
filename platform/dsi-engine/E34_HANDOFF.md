# E34 — EU ROM bed OBJ frame preview

Base: verified `e33-original-object-dispatch`. Do not merge E34 into the
stable branch until a physical emulator screenshot confirms placement/palette.

This stage binds the original EU `gObjectDefinition_4F[1]`:
GFX 99 / palette 10 / sprite 407. No handcrafted Link or furniture art.
It streams gfx from the original fixed graphics table and original ROM
gfx blob; uses the existing LZ77 bridge, EU OAM frame/anchor decoder,
and existing RGB555 sprite compositor. It displays one BedCover frame
after the real E33 original action0 sets the sprite frame. Palette bank 6
is a temporary presentation-only mapping, not game palette allocation.

The E25 room background still renders if sprite transport cannot validate
the ROM frame/palette; E34 never claims a sprite when no pixels were drawn.
This is still NOT the original full runtime sprite system and is not
playable; player sprite/animation, collisions, manager scripts and the
original GameTask updates remain disabled.

Host test checks the original compiled EU ObjectDefinition. E30-E33 host
regressions continue and ARM9 link checks verify actual graphic bridge.
No ROM asset is embedded in the NDS. Real EU ROM on SD is required.
Needs melonDS+DSi visual validation before trusting the image.

## E34 compiler checkpoint — verified

- Passing code commit: `eceeb7562641d50d7106f200a141109b6a3ee7da`
- CI run: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39715467476
- Host job SUCCESS (all E30-E33 regressions + E34 EU definition test with ASan/UBSan).
- ARM9 build and symbol provenance SUCCESS.
- NDS artifact: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39715467476/artifacts/12398379468
- Compiler logs/artifact: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39715467476/artifacts/12399160803

Visual results NOT yet confirmed. The single ROM-origin BedCover palette/sprite
overlay is experimental: no verified EU-ROM emulator screenshot. If the sprite
fails bounds or other checks, the pre-existing room screen is retained. Do NOT
promote this preview to “first playable,” and do NOT merge into main just because
CI passes. The next gameplay milestone must port native player action dispatch
and sprite draw/collision, with real VBlank frame updates and emulator checks.
