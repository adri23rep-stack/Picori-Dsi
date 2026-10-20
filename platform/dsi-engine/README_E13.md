# Engine E13 — Original START/A button -> file-select handoff (diagnostic)

## What actually changed

- `src/title.c` and `src/common.c` remain **unmodified**. The **original** `ReadKeyInput()` handles DS A/START (GBA bits 0 and 3), and `TitleTask()` handles the original fade/transition calls.
- **Fixed a blocking E12 bug:** `boot_dsi.c` previously exited the entire app when START was pressed (`0x08`). Now **SELECT** (`0x04`) exits diagnostics; START reaches the original game code.
- Added `dsi_title_runner.{c,h}`: a bounded task dispatcher that calls the unchanged `TitleTask()` only while `gMain.task == TASK_TITLE`. When original `ExitTitlescreen()` invokes `SetTask(TASK_FILE_SELECT)`, it stops **before** the unported file-selection task. The screen displays `File select: REACHED`. This is **not** a file-select menu and doesn't load or save any user data.
- Fixed old E7 host test compilation after E12 introduced original `objectDefinitions.c` and the OBJ graphics bridge.
- No ROM or extracted game assets are included in the ZIP.

## Verified on host with original Europe ROM

```
sh platform/dsi-engine/tests/run_title_e13.sh /path/to/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e13_sanitized.sh /path/to/baserom_eu.gba
```

The tests run the original GBA key register -> `ReadKeyInput()` -> `TitleTask()` pipeline. Both **START and A** reached `TASK_FILE_SELECT` after 853 scheduler ticks from the three-step initial trace; no asset, entity, VRAM or invalid GBA bus errors occurred. Six extra attempted ticks do not execute a file-select or misinterpret a task. The test also checks unknown tasks and a title tick limit.

Existing E3 through E12 tests pass, including E9–E12 sanitizers. **devkitARM and actual melonDS execution have not been run in our Linux environment**.

## Windows / devkitPro

Use this full E13 ZIP in a separate directory or extract the E13 changes into your existing `zelda-tmc-3ds-main`, overwriting files. From the project root:

```
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Output **if ARM9 build passes**: `platform/dsi-engine/tmc-dsi-engine-e13.nds`.

Keep the existing original EUROPE `baserom_eu.gba` in the melonDS DLDI/SD folder at `tmc-dsi/baserom_eu.gba`. Top: original title graphics. Bottom: diagnostics. Wait for Zelda logo, then press **START** or **A**. The diagnostic should display `File select: REACHED`; the image remains on-screen because file selection rendering is **not ported yet**. **SELECT** exits the diagnostic. No actual save file should be modified by E13.

## Remaining blocker to playable Zelda

- `src/fileselect.c` is not linked; with the current `PC_PORT` build flag it includes `port/port_ppu.h` which requires `SDL3/SDL.h` (host-side PC backend). It will need properly isolated DSi platform interfaces rather than blindly linking a PC version or stubbing a complete UI.
- `AgbMain()`, the true global scheduler, is still not running. E13 manually calls only original `TitleTask()`. Game/room/player/physics/audio/save functionality is **not connected**.
- Title fades and sound are diagnostic placeholders; hardware renderers are incomplete; no real DS hardware verification or performance measurement yet.

The goal for E14 is to compile the genuine file-selection module behind DSi adapters, isolate its first initialization task, and audit external dependencies before any save-writing is enabled.
