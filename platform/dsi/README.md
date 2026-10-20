# Zelda Minish Cap / DSi — experimental test programs

**Current version: V4 (ROM-backed real Link sprite demo, not playable).**

See **LINK_V4.md** for setup, keyboard controls, diagnostics, limitations,
and ROM offsets. The original 3DS project remains intact. Build only the
DSi subfolder using:

```sh
make -C platform/dsi clean
make -C platform/dsi
```

Expected output: `platform/dsi/tmc-dsi-probe.nds`.

Requires Nintendo DS devkitPro, libnds, libfat and the user's own clean
European `.gba` at `/tmc-dsi/baserom_eu.gba` on the configured melonDS SD.
There are no game assets in these sources or archives. See `GFX_V3.md` for
the earlier V3 tile sheet experiment, and `STATUS.txt` for test status.
