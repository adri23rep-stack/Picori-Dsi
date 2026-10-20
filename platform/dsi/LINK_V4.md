# Minish Cap for DSi — V4: real Link sprite overlay (NOT PLAYABLE)

This experimental `tmc-dsi-probe.nds` reads the user's own **clean European
16 MiB GBA ROM** from `/tmc-dsi/baserom_eu.gba`, renders a tile sheet from
it, and draws the authentic Link sprite reconstructed from the ROM's own
`gSpriteFrames_1` frame table and `gFrameObjLists` metasprite pieces.

**The game is NOT ported yet.** There are no real maps, walls, collisions,
NPCs, audio, quests, script execution, saves, or gameplay. The forest is
still an uncomposited sheet of title graphics, used as a visual background.
This is a movement/animation and ROM-read technical demo only.

## Installing and building on Windows

1. Install devkitPro's **Nintendo DS development** package (as for V3).
2. Extract `Minish_Cap_DSi_Link_V4_Solo_Cambios.zip` into the folder containing
   `zelda-tmc-3ds-main`, and replace files. The original 3DS port is untouched.
3. Open **devkitPro MSYS** and enter `zelda-tmc-3ds-main`.
4. Compile:

   ```sh
   make -C platform/dsi clean
   make -C platform/dsi
   ```

5. Copy `platform/dsi/tmc-dsi-probe.nds` to your melonDS setup. Keep the
   original **European** ROM at `/tmc-dsi/baserom_eu.gba` on the SD/DLDI
   image selected in melonDS. Reboot the emulator.

## On-screen diagnostics

The bottom screen should show:

```
SD/FAT: ready
Region: EU
Forest: LOADED
Link sprite: LOADED
```

If `Forest: LOADED` but `Link sprite: FAILED (EU only)`, first check the
cartridge image's region, then send us a screenshot. The first version of
V4's Link sprite loader is intentionally EU-only; the forest remains
available for other regions until their frame/object-list offsets are
verified.

## Controls

- D-pad: move the Link sprite over the graphics sheet.
- Hold B + D-pad: move twice as fast.
- A: change Link's tunic colors (green, red, blue, purple) from the ROM.
- SELECT: toggle the V3 graphics sheet viewer. In this mode, the D-pad pans,
  A changes its 15 original palettes, and SELECT returns to Link.
- START: exit.

## Source offsets (verified against EU clean ROM)

| Resource | European ROM file offset | Loading strategy |
|---|---:|---|
| `gSprite_Link` start | `0x13A500` | first 432 4bpp 8x8 tiles, 13.5 KiB |
| `gSpriteFrames_1` | `0x1146E0` | first 50 frames × 4 bytes |
| `gFrameObjLists` | `0x2F3460` | indexed small reads via ROM cache |
| green/red/blue/purple `gPalette_14..17` | `0x5A2590` | 4 × 16 colors |
| background title sheet | `0x8C4730` | 32 KiB |
| background title palette set | `0x5BCDF0` | 480 bytes |

The source and archives do **not** contain the ROM, Nintendo sprites or
other copyrighted game data. They are loaded directly at runtime from your
SD card. No ROM modifications are written.

## What was actually tested

- Native host C test **PASSED** using the user's EU ROM: all 50 frames × 4
  palettes draw nonempty clipped sprite pixels with no out-of-bounds writes.
- Existing V3 graphics-preview and ROM-cache/input tests **PASSED**.
- devkitARM compilation **NOT performed** in this environment (the compiler
  is not installed); running on melonDS **NOT yet verified**. Both are
  the next steps on your PC.

Optional native test command (Linux/macOS/devkitPro MSYS with a host compiler):

```sh
sh platform/dsi/tests/run_link_tests.sh /path/to/your/baserom_eu.gba
```
