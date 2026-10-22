# E31 checkpoint — native entity lifecycle

Base: `65b9b365f60ea27dfeae10f55cfc65529b871ff4`, branch
`e30-original-room-properties`. Work: `e31-native-entity-lifecycle`.
Keep the base branch unchanged. This is NOT yet playable.

Implemented: original entity deletion/recycling and its real resource cleanup,
script-context allocation/free, manager deletion, native globals, heap init,
original collision-layer lookup and packed-map row expansion. A guarded loader
executes original LoadRoomEntityList/LoadRoomEntity for the initial EU room.
It leaves the 3 managers and bed object alive alongside Link, without updates.
Unknown script records and special tiles are rejected before spawning. No GBA
callback or script interpreter is enabled. Slots 2–7 of room properties remain
blocked; InitializeEntities and GameTask UPDATE are still incomplete.

Verified locally: `run_lifecycle_e31.sh` and `run_room_spawn_e31.sh /path/to/EU.gba`
with ASan/UBSan (`E31_SANITIZE=1`, LeakSanitizer disabled due to sandbox ptrace).
The user's EU ROM yielded 3 managers + bed at original coordinates, layer 1,
Link health intact, valid lists, successful cleanup, no bus errors.
No melonDS/hardware execution has been performed in this session.

Native MemCopy now permits exact verified EntityData records (manager registry
copies), rejecting partial/misaligned/raw GBA sources. Boundary tests pass.
Actions run 39701888621 passed all preserved host regressions and E31 ASan/UBSan;
ARM9 linked and produced the NDS. Its post-build symbol gate incorrectly required
the out-of-line ClearDeletedEntity in the optimized ELF. E31 CI now checks that
function in the allocator object and its ClearAllDeletedEntities caller in the
ELF, retaining real implementation evidence without disabling optimization.
Verified build: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39702314321
at code commit f67a7c2855c61c5f06bd178f3725ebf58f48e1ec: both host and nds jobs
SUCCESS, including object/ELF symbol gates. Download Picori-DSi-E31-NDS for the
NDS + SHA256 and E31-build-evidence for compiler log, symbols and linker map.
ARM9 ELF size: text 190252, data 9924, BSS 791568 bytes (total 991744).
Build success does not establish gameplay, emulator or physical DSi success.
EU ROM test repeated successfully with ASan/UBSan after checking input is the
full 16,777,216 bytes. A transient truncated local ROM was re-materialized; do not
change engine logic to accommodate a truncated input.

Local workspace is a selected-file snapshot, not a clone of full git history.
Remote commits must preserve the upstream tree and replace only changed files.
Do not upload generated/ files or the user's ROM. The generator selects literal
engine functions; edits to original functions are limited to platform safety.

Next functional work after this checkpoint: real room callbacks and scripts,
original entity/player dispatch, sprite animation/OAM, then collision and input
updates. Do not label the persistent spawn milestone as gameplay.
