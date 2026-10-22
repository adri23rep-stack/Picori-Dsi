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

Current integration checkpoint: ARM9 build not run yet. Before calling this
ready, expand native MemCopy source allowlist to include verified EntityData
banks (RegisterRoomEntity copies manager records), update CI for E31 symbols,
run regressions, push, and wait for Actions to compile the .nds.

Local workspace is a selected-file snapshot, not a clone of full git history.
Remote commits must preserve the upstream tree and replace only changed files.
Do not upload generated/ files or the user's ROM. The generator selects literal
engine functions; edits to original functions are limited to platform safety.

Next functional work after this checkpoint: real room callbacks and scripts,
original entity/player dispatch, sprite animation/OAM, then collision and input
updates. Do not label the persistent spawn milestone as gameplay.
