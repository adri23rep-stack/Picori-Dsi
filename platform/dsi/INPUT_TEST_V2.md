# DSi Stage 0b: button diagnostic

This changes only `platform/dsi/source/main.c` in the supplied source tree.
No ROM is needed; this remains a hardware test, not a playable Zelda port.

## Rebuild (devkitPro MSYS, from project root)

```sh
make -C platform/dsi clean
make -C platform/dsi
```

Copy `platform/dsi/tmc-dsi-probe.nds` to the DSi and start it.

- **Upper screen:** red-and-white square should move using the D-pad.
- **Lower screen:** `Frames:` should continuously increase (every ten frames); `Raw:` is the actual ARM9 hardware register (pressed bits read as 0); `lib:` is libnds `keysHeld` (pressed bits 1); `hw:` is pressed bits calculated from the raw hardware register; `X:` / `Y:` track square coordinates; `D:` displays D-pad flags as U D L R. `A:Y` indicates A is pressed.
- Press A to test cache read if a locally supplied legal ROM is loaded; START quits the test.

If the square still does not move:

1. If `Frames:` **does not increase**, the program is not reaching/running the main input loop. Tell us the last line visible on the bottom display.
2. If `Frames:` increases but `Raw:` remains `3FF` and `hw:` remains `000` while holding D-pad keys, the underlying physical/emulated keypad input is not being detected; check the emulator/controller mapping or the handheld itself.
3. If `Raw:` changes but `lib:` remains `000`, the libnds polling path may be at fault. The direct hardware fallback should still move the cursor.
4. If `X:`/`Y:` changes but the square does not, investigate video/VRAM redraw, not buttons.

The direct raw register only covers the original DS A/B/SELECT/START/D-pad/L/R inputs; touchscreen/X/Y require different handling.

Note: This code was inspected and portable unit tests for the ROM reader run, but this variant could not be compiled or tested on an actual DS from this environment (devkitARM is unavailable here).
