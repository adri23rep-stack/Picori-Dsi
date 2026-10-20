# Engine E19 — Original `GameTask()` / `FinalizeSave()` bring-up (PC host)

## Actual, reproducible result

The E19 PC integration test builds unmodified `src/game.c` and
`src/playerUtils.c` plus `src/save.c`. To link the **unchanged**
`FinalizeSave()` without dragging every other player function into this
limited test, the test script applies **`ld -r --gc-sections -u FinalizeSave`**
to the original compilation output. This is a *test-only link stage*, not a
rewritten copy of `FinalizeSave()`.

Using the European ROM, the PC test:

1. Invokes the real RAM-only `ResetSaveFile()` and `SetTask(TASK_GAME)`.
2. Calls the real `GameTask()` with fade active: it increments the frame
   count and waits in `GAMETASK_TRANSITION`.
3. Clears fade and calls the real `GameTask()` again. Its actual
   `GameTask_Transition` invokes the genuine `FinalizeSave()` from
   `src/playerUtils.c`, initializes the new save (LINK, HP, etc.), copies
   saved player status to `gRoomTransition`, and enters `GAMETASK_INIT`.
4. Calls the real `GameTask()` a third time. The original
   `GameTask_Init` places area `0x22` / room `0x15` into the room controls
   and transitions to `GAMETASK_MAIN` / `GAMEMAIN_INITROOM`.

**No first room is loaded**. This stops *before* executing
`GameMain_InitRoom()`. The true target data from the new-save initialization
is area `0x22`, room `0x15`, position `0x90, 0x38`.

## Host-test service shims — major limitation

Many `GameTask_Transition`/`GameTask_Init` calls are not integrated:
UI and room cleaning, game sound reset, allocator init, graphics groups,
physics reset and tilemap clear are **counted test-only shims**.
Other unported gameplay functions are linked to `abort()` **only in the
host test**, so attempting a later state would fail loudly rather than
falsely behaving like gameplay. No saving to filesystem is performed.

The whole ROM is **never bundled** in these ZIPs. PC tests need your own
EU 16-MiB dump. Game ROM stays on the DSi SD for native runtime.

## Nintendo DSi executable

E19's native build adds `dsi_game_transition.[ch]` and reports a bounded
RAM handoff **preflight** once you press X after entering file select.
This is intentionally NOT the original `GameTask()` call on hardware.

The source-copy resolver now whitelists the original save-status object
rather than rejecting that native pointer. It still refuses arbitrary GBA
memory addresses. Older E17 tests link fine without the optional E19 bridge.

The native build has not been compiled with devkitARM or run in melonDS
here. Do not claim a running room or playable Link on this evidence.

### Compile in devkitPro MSYS

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Expected file: `platform/dsi-engine/tmc-dsi-engine-e19.nds`.
Use the usual EU ROM at `fat:/tmc-dsi/baserom_eu.gba` (or `sd:/...`).
At the original title press START or A, and then X after the file-select
handoff. Native diagnostics: `Game RAM: READY` and `Transition: INPUTS OK`
mean the preparation check succeeded, **not that GameTask executed**.

### Repeat PC integration and regression tests

```sh
make -C platform/dsi-engine test-e19 ROM=/path/to/baserom_eu.gba
make -C platform/dsi-engine test-e19-sanitized ROM=/path/to/baserom_eu.gba
make -C platform/dsi-engine test-all-e19 ROM=/path/to/baserom_eu.gba
```

AddressSanitizer/UndefinedBehaviorSanitizer instrument the E19 harness and
adaptors; most original modules are compiled without sanitizer flags.

## Next hard blocker

Integrate the *real* services used in `GameTask_Transition`,
`GameTask_Init`, and `GameMain_InitRoom` on DSi. In particular: memory
allocation/reset, room graphics groups and map loading, player
initialization, camera, collisions, and audio calls (initially safe mute).
Add direct `GameTask()` native invocation **only after** those services
have been audited and the ROM-pointer/memory model is proven safe.
