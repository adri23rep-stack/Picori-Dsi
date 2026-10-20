# Stage 1 — ROM-based title graphics sheet (v3, NOT PLAYABLE)

Tested on the **clean, 16 MiB European GBA ROM** (SHA-1
`cff199b36ff173fb6faf152653d1bccf87c26fb7`) using a local offline
renderer and portable unit tests. **Not yet built with devkitARM here** and
**not yet tested in melonDS or physical hardware**.

## What you will see

- The top 240x160 viewport displays **original, uncompressed 4bpp tiles**
  from title graphics group 2 (`gGfx_2_0`), colored using the western title
  palette group 4. Initial view starts with a forest background. The bottom
  screen confirms the ROM path, SD mount and preview status.
- This is a **raw tile atlas**, not the GBA's full title screen: the real
  scene needs background tilemaps, multiple layers, sprites, animation and
  sound. Some areas in the atlas are expected to look like fragments or
  stripes due to their original tile-sheet ordering.
- Controls: D-pad pans the tile sheet, A cycles 15 actual palette sets,
  B toggles the original movable red cursor diagnostic, START exits.
- ROM data is loaded into a static ~33 KiB RAM buffer (not the full ROM).
- No copyrighted game assets or ROM data are included in the package.

## Windows 11 / devkitPro MSYS

From `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi clean
make -C platform/dsi
```

Copy `platform/dsi/tmc-dsi-probe.nds` to your melonDS DSi SD card.
Place your own **clean EU** ROM at `/tmc-dsi/baserom_eu.gba` on the DSi SD
(or the DLDI FAT volume configured in melonDS) and reboot the emulator.

Expected lower-screen output includes `SD/FAT: ready`, `Title sheet: LOADED`,
then a moving `Frames` counter. If missing, confirm the **same SD** is
configured for the homebrew and the card was synced/reloaded.

## Reference offsets

The known-clean EU ROM uses:

| Asset | EU file offset | Bytes |
|---|---:|---:|
| `gGfx_2_0` original title sheet | `0x8C4730` | `32768` |
| title palettes group 4, 15 × 16 colors | `0x5BCDF0` | `480` |

USA locations are provisionally supported from `assets/gfx.json` for future
checks: sheet `0x8C4F60`, palettes `0x5BD8E0` (not checked against USA ROM).

The helper `tmc_gfx_preview.c` can be host-compiled without libnds:

```sh
sh platform/dsi/tests/run_gfx_tests.sh
sh platform/dsi/tests/run_host_tests.sh
```

## What is NOT implemented

No title background map compositing, no title text overlay, no Link sprite,
no GBA emulation, no gameplay, no sound and no save system. Displaying a real
ROM graphic does not mean the 3DS engine has been ported.
