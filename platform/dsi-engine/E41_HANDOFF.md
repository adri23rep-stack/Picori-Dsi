# E41 original interior collision

Base E40: 4285796d17214a11bbb0a35f7300afe90eda725b. Branch:
e41-original-interior-collision. E40 and older branches remain unchanged.
NOT PLAYABLE. Do not force CONTROL_ENABLED or invent player movement.

Implemented: native original sub_0807BBE4 (playerUtils.c), followed by original
border routine and FillActTileForLayer (scroll.c). Both 4096-cell layers get
their real collision and action-tile bytes, and mapDataOriginal is preserved.
The generator extracts literal source bodies; ARM9 uses existing playerUtils.c.

Tables are read only from the user's EU ROM, through bounded bus transport.
Provenance: asm/src/veneer.s SetTile literal pool plus EU header offset 0x48:
0x08000258 -> 0x080b7038 special act, 0x25c -> 0x080b70cf special collision,
0x260 -> 0x080b2ec8 regular act, 0x264 -> 0x080b35a8 regular collision.
The complete native act table (1396 types) is compared byte-for-byte to ROM
before any map mutation. Special types 0..150 are accepted; every cell of both
maps is bounds-checked first. Unknown types, foreign pointers and short reads
stop preparation. No success stubs or baked ROM tables added.

Tests passed locally under ASan/UBSan: run_collision_e41.sh (all regular/special
types, malformed indices, pointer/read failures, act mismatch, no partial map
writes, idempotence), run_room_spawn_e31.sh with the complete user's EU ROM.
Real ROM test independently compares all 8192 collision and act entries to ROM
plus original borders. Bedroom interior totals: 113 blocking, 187 empty cells.
E40 graphic quarantine, original Archway, Link origin and static sprite retained.
CI pending at first checkpoint; no melonDS/hardware test in this session.

Next blockers: original PlayerInit/sleep/wake dispatch, animation/OAM pipeline,
room callbacks/scripts/managers, original movement/collision queries, transitions.
Full collision arrays alone do not run the collision solver or make Link playable.
Do not confuse object 0x4f Archway with BedCover 0x5b (old E31 wording was wrong).
Local repo remains a selected-file snapshot; remote tree must be preserved.
