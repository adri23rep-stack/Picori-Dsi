# E12 — Original title-object graphics on DSi (EU ROM)

## What really runs

- `src/title.c::TitleTask` **unmodified**, as in E11.
- `src/object/titleScreenObject.c` **unmodified** for title-entity updates.
- `src/objectDefinitions.c` **unmodified** and linked for authentic EU `gObjectDefinition_BD[]` sprite definitions.
- Native `dsi_title_object_gfx.c` bridge reads the EU fixed-GFX table (0x08132180) and OBJ-palette table (0x081329B4), follows offsets into the original ROM, and copies validated bytes to GBA VRAM/palette shadow.
- Sprite placement and frames come from the original entity state and the ROM frame-OBJ tables. The bounded DSi CPU renderer composites the original graphic onto the title background.

**This is a limited title-scene bridge, not `ObjectInit`, a complete allocator, nor playable Zelda.** Some unsupported animation/fade/audio calls remain diagnostic hooks. `AgbMain` is NOT called. No game ROM is included.

## Host verification (GCC/Clang)

From project root with an **original EU ROM** located outside the source project:

```
python3 platform/dsi-engine/tools/generate_offsets_from_json.py
sh platform/dsi-engine/tests/run_title_e12.sh /path/to/baserom_eu.gba /tmp/e12_title.ppm
sh platform/dsi-engine/tests/run_title_e12_sanitized.sh /path/to/baserom_eu.gba
```

Generated `build/EU/assets/{map,gfx}_offsets.h` store only constants derived from the project's JSON metadata, not game content. They are included in this package so Windows can compile without running the original asset extractor first.

Verified on the owner's EUR (BZMP) ROM by the host suite:
- Four original title graphics records: IDs 511–514, exactly 9,216 bytes loaded to GBA shadow OBJ tiles, with first/last bytes compared to the ROM.
- Three original palette records: IDs 359–361, 96 bytes compared in full to the ROM.
- Title frame pixel hash `870d8f7f` in the host test after original title/objects advance, displaying Zelda logo over forest.
- E3–E11 host regressions and E12 ASan/UBSan path pass.

## Compile on user's devkitPro MSYS (NOT validated here)

```
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Expected (if devkitARM build succeeds): `platform/dsi-engine/tmc-dsi-engine-e12.nds`. Launch in melonDS with DSi + FAT SD enabled and original file at `fat:/tmc-dsi/baserom_eu.gba` (or `sd:/tmc-dsi/baserom_eu.gba`). Do not mistake host rendering success for DSi hardware execution; cross-toolchain not available in our runner.

### Limitations

- Bounded **title-only** 8-entity bridge. The game's general `ObjectInit`, `LoadFixedGFX`, palette allocator, proper 3DS/GBA audio and save services, all room/entity logic, and `AgbMain` still need work.
- The current software compositor handles ordinary OBJ pieces, not every GBA affine/priority/window case.
- E12's graphic slot assignments follow original fixed-GFX slot arithmetic in a small monotonic allocator. They are not the game's full active-slot compaction or reference counting system.
- The PC test advances title scripts with diagnostic no-op fade/audio; game entry is not yet connected.
- No ROM or copied textures are redistributed; the user's ROM is read from SD at runtime.

## Provenance

EU addresses are the existing `kRomOffsets_EU` values in `port/port_rom.c`; object definition IDs and type data come from original `src/objectDefinitions_data.inc`. No edits to the original game modules are necessary for E12.
