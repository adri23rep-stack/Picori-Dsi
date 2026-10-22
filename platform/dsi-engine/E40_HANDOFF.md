# E40 — Clean authentic room, quarantined corrupted original-Archway compositor

The user's **successful melonDS E39 screenshot** clearly shows:
- The original EU room background rendered at original map coordinates.
- Link's ORIGINAL ROM sprite finally visible at original bedroom bed.
- `Arch: ACTIVE T:1 E:0`, `Link original: LINKED`, `org:1 col:1 linkR:0`.
- A **high-saturation magenta/black rectangular graphic above the upper
  doorway**, introduced by the provisional E39 `DsiRoomArchwayGfxE39_Draw`
  implementation, not present in E37/E38 screenshots.

E39 already has source-correct original `Archway()` and `InitializePlayer()`
position. Do not regress either. The erroneous component was the *separate
experimental GBA-OBJ graphics preview*, which copies original bytes to a
manually-chosen VRAM/palette bank without the game's normal
`ObjectInit/LoadFixedGFX/LoadObjPalette` allocation + real OBJ scheduler.
Provenance of individual ROM bytes does not prove correct rendering.

Changes:
1. Quarantine publishing the unverified Archway preview by an explicit
   source testable E40 sprite policy. The ORIGINAL `Archway()` still runs
   and the original room BG1/BG2 pixels remain visible in its place.
   No fake room tiles/sprites, and no changes to ROM object data.
2. Correct the packed fixed-gfx source pointer in the experimental module,
   matching original `src/vram.c::sub_080ADDD8` `record & 0xFFFFFC`.
   This does **not** make the experimental OBJ compositor production ready.
3. Bottom screen `ROOM:DISPLAY ARCH:HOLD` intentionally reports that
   the preview is blocked; `Arch:ACTIVE` remains separately reported.
4. Show authentic player action, subAction and spawn_type on row20 to
   prepare porting the original sleep->wake action dispatcher. Link still
   draws its **static ROM frame 0**; its real state updates are NOT running.
5. Host test asserts Archway ID 0x4F and BedCover 0x5B, preservation of
   genuine entity while graphics blocked, source C order and flags mask.
   All previous regressions E30-E39 retained.

No melonDS execution in CI; request screenshot of E40. Do not promise
actual playability: player action dispatch, sleep scripts, interior
collision and room transitions still require native source integration.

## Verificación de compilación E40 (22 oct 2026)

- Commit del código probado: `739e7190e1e4efe5a5a0769f7e8efcb276b3b22c`.
- CI completo: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39829385482
- Host E30–E40: **PASS**, incluyendo regresiones, ASan/UBSan y orden de composición.
- Compilación ARM9 con devkitPro: **PASS**; símbolos de rutina original y puerta de render comprobados.
- NDS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39829385482/artifacts/12448995094
- Logs y símbolos: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39829385482/artifacts/12447769288

### Para la siguiente captura

- Comprobar que Link continúa visible en la misma cama.
- Comprobar que desapareció la franja rosa y negra sobre la entrada.
- `Arch: ACTIVE` significa que ejecutó la acción original.
- `ROOM:DISPLAY ARCH:HOLD` es deliberado: la *vista gráfica experimental* del arco está suspendida.
- La fila `a:<n> sub:<n> spawn:<n>` revela el estado original del jugador, útil para preparar el motor real de despertar.
- Esto todavía no demuestra control jugable ni transición de habitación.

No añadir gráficos creados a mano ni enlazar accidentalmente el compositor anterior en producción.
