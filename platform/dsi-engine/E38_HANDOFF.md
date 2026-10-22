# E38 — Fix from user's E37 melonDS photo (2026-10-22)

User's screenshot: authentic room pixels and 3 managers/2 entities, but
`Bed object: BLOCKED O:0` and no visible Link. All host tests/ARM9 passed
E37, but they did not reproduce the ACTUAL native order or show failures.

Two bugs found by inspecting `boot_dsi.c`:
1. `DsiRoomSpawnE31_Load` ran **before** E37 bound original
   `gRoomControls.origin_x/origin_y`. Original `src/room.c`
   `sub_0804AF0C` adds those offsets to EntityData coordinates,
   therefore the real room object spawned at the WRONG WORLD position.
   E38 binds the independent real InitRoom RoomResInfo and EU ROM metadata
   BEFORE `LoadRoomEntityList`. Reuses original code, no altered game rules.
2. E34 bed overlay checked `DsiRoomObjectE33_Ready()` **before**
   E33 could even execute `BedCover`. The renderer has moved after
   the native bed tick, before Link.

E38 adds noninvasive diagnostic reason codes and screenshots-friendly
lower screen rows 18-20 that persist during title counters:
- Link world x/y and ROM draw failure status.
- Bed flags/action/world x/y; `Bed:... E:n` reason on row16.
- geometry/collision boolean state and original Link ROM read failures.

E33 reason codes: 1 not ready, 2 task/entity count, 3 missing bed,
4 wrong type, 5 scripted/deleted, 6 invalid action, 7 changed bed,
8 original priority suppressed, 9 genuine action postcondition failed.
E35 reason: 1 original-room/player guard, 2 offscreen world coords,
3 cannot stream verified original sprite from EU ROM.

Still NOT first playable: unported original Link normal/wakeup/movement,
interior collision and original game sprite/manager dispatch. Testing
needed with same ROM in melonDS; compare E34 working build if regression.
Never upload the ROM to GitHub.
