# Minish Cap DSi Engine E23 — First native GameTask boundary

## What changed

- `src/game.c` still contains the original `GameTask_Transition()` and `GameTask_Init()` algorithms. The new `TMC_DSI_SAFE_GAME_BOOT` compile flag **only** guards its dispatcher on DSi, preventing calls to unported later states.
- On X after reaching the file-select diagnostic, `dsi_game_native.c` calls the **original** `GameTask()` twice: once for TRANSITION and once for INIT, if the original RAM-only save state passes validation.
- `FinalizeSave()` is the real function from `src/playerUtils.c`. Its original constant `gUnk_0811E470` is compiled from `src/save.c`. No disk save or file-select task is invoked.
- The missing audio, UI, collision and room services are **explicit deferred counters**, not implementations. The adapter STOPS at `GAMETASK_MAIN / GAMEMAIN_INITROOM`; this is a diagnostic, **not gameplay**.
- Earlier EU room maps and tileset preflight remain after the two safe states.

## How to compile (Windows devkitPro MSYS)

From `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

The intended output is `platform/dsi-engine/tmc-dsi-engine-e23.nds`.
The test needs your own unmodified **European GBA ROM** in the melonDS DLDI SD folder at `tmc-dsi/baserom_eu.gba`.

In melonDS, allow the original title to appear, press START or A, and when the title has handed off, press X once. On the bottom screen look for:

```
GameTask: MAIN WAIT
orig:1+1 deferred:10 err:0
Room ROM: VERIFIED
Room tiles: OK ...
```

It is normal for the TOP screen to retain the title graphics. **The game room is not rendered** and Link cannot move yet. `MAIN WAIT` does **not** mean the main gameplay loop works. No frame of `GameMain_InitRoom` is executed.

## Host verification

```sh
make -C platform/dsi-engine test-e23 ROM=/path/to/your/own/baserom_eu.gba
make -C platform/dsi-engine test-e23-sanitized ROM=/path/to/your/own/baserom_eu.gba
```

The sanitizer build instruments the new DSi transition adapters and test harness. The large upstream source modules are not instrumented in this suite, because instrumenting those unused functions retains unlinked features and falsely pulls hundreds of unresolved symbols into the host test. This is not a full-game sanitizer certification.

## Limitations and next step

- E23 native ARM9 build and actual melonDS run **not verified in this environment** (devkitARM missing).
- `AgbMain()`, `GameMain_InitRoom()`, player collision, audio, persistent saves and gameplay loop remain unported.
- The next task is replacing the deferred room services with authentic functions and wiring `GameMain_InitRoom` gradually, always maintaining a fail-closed barrier.
- `src/game.c` has one conditional dispatcher change for this native safety boundary; the original state logic is unchanged outside that guard.
