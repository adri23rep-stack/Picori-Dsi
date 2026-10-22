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
