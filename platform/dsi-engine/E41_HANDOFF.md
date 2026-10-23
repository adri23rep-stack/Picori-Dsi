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
E41 CI passed: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39849021718
Code commit: 795ab9c67c869c576cb81e46a0b1754961ab7c3f.
No melonDS/hardware test in this session.

## E41.1 / E42 pixel-collision dependency block

Existing original GBA-to-C translations in port_linked_stubs.c now supply the
collision query family, SelectPlayerCollisionTable, TileCollisionLookup and
sub_080085CC (eight hitbox probes). Bodies are selected verbatim. Only data and
address-resolver symbol names are bound to native DSi transport; no new rules.
The seven 96-byte parameter banks are read from EU 0x08008374 (assets.json's
USA 0x82dc + regional 0x98); all 40 16x16 masks use the region-correct packed
pointer table PORT_COLLISION_SHAPE_PTRS_EU = 0x82d4 from port_config.h.
No raw GBA pointer becomes a native address and no missing mask returns fake
passability. A probe requires the actual gPlayerHitbox; this block does not
install it early or move Link. The boot path only checks a pixel at Link's real
position. PIXEL on row19 indicates data+original point lookup, NOT gameplay.

Sanitizer tests pass for every byte-valued collision type, every pixel, seven
parameter modes, invalid/missing masks, layer selection, world-origin hitbox
offsets and all eight edge bits. Full EU ROM integration independently compared
every bedroom pixel on both layers in eight player states against raw ROM masks.
E41.1 CI passed: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39849729718
Code commit: 63012aef3cbb03a679dd9f631beeb31e405e13c7.
Workflow retains E30-E41 regressions and adds E42.

## E42 original movement dependency

The generator now also selects CalcCollisionDirectionOLD, LinearMoveDirectionOLD
and sub_0800857C from the existing port translation. Original FixedMul is selected
from physics.c and sineTable.c is compiled directly. Direction masks are decoded
from the original gUnk_0800275C assembler bytes, with exact size checking.
The two signed negative left shifts in the existing C translation use *256:
same GBA position scaling, defined C behavior for all s16 FixedMul results.
No player frame dispatch or diagnostic movement has been enabled.

Host sanitizer tests cover all 65536 collision masks for each cardinal direction
(262144 wall-slide cases), all 32 directions/opposites, stopped/enclosed entities,
all signed FixedMul inputs at 11 speeds, and the real hitbox-refresh wrapper with
its original airborne/type2 exclusions. Native CI checks movement object symbols;
unreferenced movement is correctly removed from the final ELF until actual
player dispatch is ready. Do not claim this dependency as playable movement.

## E42 complete UpdatePlayerMovement dependency closure

First movement CI 39884223678 passed host regressions but caught conflicting
legacy asm.h/physics.h return declarations on ARM9. Both now agree with the
existing C port's u32 return under PC_PORT; original GBA void ABI is preserved.

Native playerUtils.c now binds its original parameter banks and shape lookup to
the same verified E42 ROM transport. Its UpdatePlayerMovement/sub_08079E58 and
direction/slope/depenetration functions are unchanged. Host tests select those
literal bodies plus their original gUnk_0811C118 table. The extractor now matches
top-level function declarations explicitly, never a call inside an if statement.
Tests cover the original direction corrections, every free-space direction,
speed modifiers/minimum/zero, and all 40 shapes on all four correction sides
at every interior pixel while retaining fractional coordinates.

The linker now explicitly retains both original movement entry points and CI
requires the complete closure in the final ELF, to detect missing dependencies.
This supersedes the previous garbage-collection note above. Neither entry point
is called by boot/input: original PlayerInit/PlayerSleep/frame dispatch remains
the next blocker. Preconditions remain initialized hitbox/maps/ROM parameters.

Closure CI passed: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884554411
Code: f0fea5aff7572b9efbd5556301782fc20682a474. Host regressions and final
ARM9 movement symbols both passed; a real NDS was produced.

Following that build, a stopped-player regression reproduced a native global
buffer overflow: original idle/jump paths can pass DIR_NONE (0x80) through
sub_08079F48 before LinearMoveDirectionOLD checks it. PC_PORT now bounds-checks
each slope-table index, preserving all valid comparisons and returning FALSE
when no slope entry exists. Original GBA build is unchanged. Tests cover every
byte direction/collision pair and stopped UpdatePlayerMovement under sanitizers.

Full EU ROM integration also passed 10824 original UpdatePlayerMovement steps
at 328 clear room positions across both layers (all 32 directions plus stopped),
with exact fixed-point displacement and the persistent player restored afterward.
These are host tests only, not a new input-driven runtime path.

Next blockers: original PlayerInit/sleep/wake dispatch, animation/OAM pipeline,
room callbacks/scripts/managers, original movement/collision queries, transitions.
Full collision arrays alone do not run the collision solver or make Link playable.
Do not confuse object 0x4f Archway with BedCover 0x5b (old E31 wording was wrong).
Local repo remains a selected-file snapshot; remote tree must be preserved.
