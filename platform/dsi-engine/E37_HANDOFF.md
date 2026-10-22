# E37 — actual original room header geometry enables E36 collision border

Root cause fixed: the guarded Zelda `GameMain_InitRoom` deliberately never
calls `InitializeCamera`. Hence after game boot `gRoomControls` had no
`width/height/origin_x/origin_y`, and E36 always returned BLOCKED.
Compiling the E36 `.nds` did not catch that runtime preflight mismatch.

E37 ties those fields to the REAL original `RoomResInfo` produced
by genuine `InitRoom` and validated independently against EU ROM metadata.
This is a narrow relocation/binding of upstream source state, not a recreated
camera or game engine. Conflicting partial nonzero state fails closed;
no writes before full validation. The original `CreateCollisionDataBorderAroundRoom`
is called without modification immediately after. Host tests deliberately
start with zero geometry, exercise contradictory originals, verify both
original collision layers, and reset/restart. E30-E35 regressions retained.

CAVEATS: The interior tile collision LUT, ORIGINAL wake-up/PlayerNormal
and animations have not been ported, so Link remains static and player
input doesn't drive physics. No external EU ROM is bundled, and melonDS
validation is still needed. User-verified E34 remains untouched.
