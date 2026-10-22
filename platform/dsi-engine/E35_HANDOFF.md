# E35 — Native collision arrays and original bed visible after spawn

Base: user-confirmed E34. Two direct port fixes:

- E34 tried to composite the real BedCover frame **before** entity spawning,
  so the required E33 original action0 could never run in time. E35 calls
  `DsiRoomBedGfxE34_Draw` only after the genuine persistent room spawn and
  first BedCover tick. Original EU graphics remain loaded from the user's ROM;
  no art is embedded in .nds.
- E31 copied only three of the original GBA MapLayer arrays. E35 copies
  collisionData at offset 0x2004, mapDataOriginal at 0x3004, tileIndices at
  0x6004 and actTiles at 0xb004, in addition to the original map arrays.
  The ROM shadow may still have uninitialized collision data because the
  full original room generation callbacks are NOT active.

The unchanged GBA ARM/Thumb collision lookup port source provides four
original functions: `GetCollisionDataAtTilePos`,
`GetCollisionDataAtWorldCoords`, `GetCollisionDataAtEntity` and
`GetCollisionDataRelativeTo`. These are generated literally from
`port/port_linked_stubs.c`. A sanitized host regression verifies layer
selection and the exact accessor results. A DSi READ-ONLY audit reports
the number of nonzero collision tile entries; it refuses to claim ready
if all entries are zero.

Movement/PlayerNormal, animation, room scripts, camera and exits are NOT
implemented or enabled yet. Never treat nonzero map bytes as proof of
collision behavior. Real melonDS/hardware visual testing remains required.
