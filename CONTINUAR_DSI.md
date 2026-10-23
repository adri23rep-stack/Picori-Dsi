# E50–E53 EXPERIMENTAL: original player, terrain and animation ROM tables

**Current branch:** `e50-original-tile-player-bindings` (base:
`e46-player-init-runtime-lab`). Do not use `main` or
`e45-original-player-init-object` for experimental player changes.
Keep E45 as the last verified diagnostics NDS.

## Progress saved for another developer / AI
1. ARM9 `PlayerInit` and genuine `src/player.c` dispatcher remain forcibly
   linked in this experimental branch, which intentionally causes red CI
   until ALL original dependencies are implemented. Do **not** add dummy
   success services, fake Link movement, or silence missing references.
2. E50: original 3DS port's tile-type / action tile lookups, the actual
   `FindValueForKey` table search and the original static world map
   conversion helpers. Desktop-only input/telemetry/Reborn code excluded
   solely under `TMC_DSI`; desktop builds unaffected.
3. E50.1: original `CheckNEastTile` and
   `PlayerCheckNEastTile` bodies, not invented collision rules.
4. E51: exact `gUnk_080082DC` symbol alias to E42's *real EU ROM*
   seven 96-byte collision parameter banks. No baked/fake data.
5. E52: source-authentic `GetTileIndex`,
   `GetLayerByIndex`, `sub_0806F730` (original hitbox-relative
   leading tile lookup). Original `sub_0806F948` deliberately deferred:
   its unresolved C-direction enum names cannot be guessed.
6. E53: native EU ROM gSpritePtrs table read, using the verified EU
   offset 0x002A5C from the PC port's `kRomOffsets_EU`. Reads
   the original packed 16-byte sprite definition and follows the
   animation index to actual frame bytes through DSi ROM page API.
   Native boot preflights Link sprite index 1, animation index 2
   (exact values from original `PlayerInit`). Does NOT dispatch
   animations or move Link yet. Host synthetic-pointer test uses
   ASan/UBSan; no copyrighted ROM committed.

## Evidence & blockers
E49 first forced native link: **67 distinct unresolved references**.
E50 native link (run 39909711401): **52 distinct unresolved references**.
The 15 reductions are verified via real ARM9 linker logs, not gameplay
claims.
Later E50–E53 native link (run 39911093009): **47 distinct unresolved
references**; 20 fewer than E49. Host tests succeeded, including E53
ROM pointer validation under sanitizers. ARM9 compilation reaches the
linker, then fails on missing real engine functions. Full details:
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39911093009
 Latest E51–E53 totals must be read from the latest Actions
run before reporting. Main missing services include actual original
`UpdateAnimationSingleFrame`, `InitAnimationForceUpdate`,
`sub_080042BA`, `ExecuteItemFunction`, scripts, GBA OAM/VRAM,
effects, game callbacks and room transitions.

**NOT PLAYABLE.** No working player action tick, wake-up script,
animation renderer or verified hardware execution. Keep the proven E45
NDS downloadable until the complete native link and melonDS test pass.
Do not bundle user GBA ROM. See detailed `platform/dsi-engine/E46_RUNTIME_LINK.md`
and GitHub Actions on current branch.

---

# EXPERIMENTAL E46–E49 — ORIGINAL engine integration

**Continue here:** `e46-player-init-runtime-lab`, NOT default `main`.
Keep preserved stable branch `e45-original-player-init-object` intact.
Original DSi goal: native port of Zelda's existing game source, no custom
Link movement, no fake wake-up script, no hardcoded GBA-ROM data committed.

## Code actually added

- E46: retains original `PlayerInit` at link time and builds entire
  upstream `src/player.c` (real `DoPlayerAction` / `PlayerSleep` dispatch).
- E46.1: compiles upstream `itemDefinitions.c`, `itemUtils.c`,
  `itemMetaData.c`, `mapActTileToSurfaceType.c`.
- E47: exact `sub_0806FDA0` extra-sprite allocation and
  `sub_0806FD8C` reset from original `src/physics.c`;
  references existing E31 original memory bank, plus Link clone pointers.
- E48: upstream `src/vram.c` original sprite GFX slot manager and
  `LoadSwapGFX`. Duplicate native services excluded only for TMC_DSI.
- E49: source-authentic `FixedDiv`, `LinearMoveUpdate` and existing
  port translation of `GravityUpdate`, compiled for ARM9.

Generator checks compare extracted function bodies against upstream source.
All additions are experimental. No gameplay frame dispatch enabled yet,
because the dependency closure does not link. A green host job or an
existing older NDS does NOT imply the experimental runtime is playable.

## Outstanding engine blockers

- Original animation state machine and native ROM SpritePtr/frame resources:
  `InitAnimationForceUpdate`, `UpdateAnimationSingleFrame` etc.
- Full player/manager callbacks, sleeping scripts, `ExecuteScript`,
  scene transition, surfaces, effects and original collision queries.
- Desktop-only 3DS extras (`Port_SoftSlots`, `Port_Reborn`, logging):
  exclude these **only under TMC_DSI**, preserving behavior on desktop.
- GBA VRAM/OAM to libnds transport, and reliable ROM graphics/animation
  palettes. Current visible Link is still a static initial sprite.
- Validate actual game initialization in melonDS, then real DSi, not only
  compilation and tests.

## Verification

Latest successful complete host tests before source-collision fixes:
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39907925209
Native ARM9 experimental link was red there due multiple definitions and
remaining original modules. The duplicate ownership of extra-sprite
state and VRAM services has since been corrected in later commits.
Check the **latest** workflow before claiming any new status.

Last known successful downloadable diagnostic NDS from E45:
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39906400123
This NDS is NOT playable. The experimental E46–E49 branch must not publish
a NDS as usable until native linker and runtime actually work.

Detailed provenance: `platform/dsi-engine/E46_RUNTIME_LINK.md`.
Never upload the user's copyrighted EU ROM or generated assets.

---

# Continuación del port nativo DSi

Rama conservada previa: `e45-original-player-init-object`. Para experimentar, usar E46–E49 arriba.
Base conservada: E40 `4285796d17214a11bbb0a35f7300afe90eda725b`.
Último código y pruebas de esta sesión:
`8a09f0a4d035d2414b3cd7d12d234a4deb1888b8`.
Detalles y procedencia: [E41_HANDOFF](platform/dsi-engine/E41_HANDOFF.md).

## Reglas del usuario

- Portar el código original; no recrear movimiento, scripts ni gameplay.
- Guardar cada avance coherente en GitHub para que otra IA pueda continuar.
- Explicaciones breves; no forzar CONTROL_ENABLED ni simular servicios pendientes.
- La ROM europea es del usuario: nunca subirla al repositorio.

## Lo que ya está implementado

- Mapas de colisión y actTiles completos de ambas capas, con rutinas originales.
- Parámetros y máscaras por píxel leídos de la ROM EU con límites comprobados.
- Sondeo original de los ocho puntos de la hitbox.
- LinearMoveDirectionOLD, FixedMul y seno originales; deslizamiento contra paredes.
- UpdatePlayerMovement y sub_08079E58 enlazados en ARM9 junto con sus dependencias
  originales de dirección y corrección de penetración.
- Corrección de seguridad PC_PORT: DIR_NONE provocaba acceso fuera de la tabla de
  pendientes. Se reproduce con ASan; ahora cada índice está limitado.

Esto todavía NO es jugable. El boot no llama a esas entradas de movimiento.
La habitación y Link estático siguen usando la ruta E40/E41 existente.

## Validación

- CI del enlace completo: [39884554411](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884554411), aprobado.
- CI de la corrección DIR_NONE: [39884783443](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884783443), aprobado.
- CI del último checkpoint: [39884902430](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884902430), aprobado en host y ARM9.
- ASan/UBSan: 262144 combinaciones de paredes cardinales, 40 máscaras y cuatro
  lados de corrección, velocidades con signo, direcciones y estado detenido.
- Con la ROM EU local: todos los píxeles de ambas capas en ocho estados y
  10824 pasos reales de UpdatePlayerMovement en 328 posiciones despejadas.
- No se ha probado este avance en melonDS ni en hardware.

Comandos de host desde la raíz:

```sh
E42_SANITIZE=1 sh platform/dsi-engine/tests/run_collision_probe_e42.sh
E31_SANITIZE=1 sh platform/dsi-engine/tests/run_room_spawn_e31.sh /ruta/ROM_EU.gba
```

## E44 — compilar src/player.c original para ARM9, sin fingir jugabilidad

Se añadió `platform/dsi-engine/tools/compile_player_e44.sh`, que
compila íntegramente el `src/player.c` original como objeto ARM9, con las
mismas definiciones de plataforma y optimización que el motor DSi.
La comprobación exige que existan `DoPlayerAction`, `PlayerInit` y
`PlayerSleep` reales, y deja una lista de referencias todavía sin resolver.
**NO enlaza ese objeto al .nds, no llama ninguna acción y no activa controles.**
Así se pueden corregir dependencias sin romper el arranque estable.

CI: rama `e44-original-player-arm9`, workflow `dsi-e41.yml`.
Los archivos `e44-player-original-symbols.txt`,
`e44-player-unresolved.txt` y `e44-player-link-gap.md` forman parte de
`E41-build-evidence`, si las pruebas pasan. La última comparación
distingue símbolo en ELF de comportamiento completo.

## E45 — PlayerInit original incluido en la compilación DSi (no activo)

El generador `generate_player_init_e45.py` selecciona el cuerpo **literal**
de `PlayerInit` en `src/player.c`, conservando su lógica y llamadas originales.
`Makefile` ahora compila `dsi_player_init_e45.c` junto al motor.
La nueva entrada `DsiPlayerE45_OriginalInit` queda fuera del ELF final por
`--gc-sections`: su ejecución todavía necesita dependencias originales reales
(animaciones, cargas GFX, habilidades, macros de jugador). El CI exige
tanto la presencia del objeto como su ausencia en el ELF ejecutable:
**nada de movimientos simulados ni habilitación prematura**.
Prueba host: `sh platform/dsi-engine/tests/run_player_init_e45.sh`.

CI completo: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39906400123
**host SUCCESS / nds SUCCESS**, incluyendo compilación real del archivo
`dsi_player_init_e45.o` y verificación de que la función aún no se ejecuta.
Artefacto actual (diagnóstico, NO jugable):
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39906400123/artifacts/12480981696

## Siguiente trabajo real

Ya hay una revisión automática para orientar el siguiente bloque:

```sh
python3 platform/dsi-engine/tools/audit_player_e43.py
```

Compila el player.c original y sigue sus referencias, incluidas las tablas
de acciones de PlayerSleep. El CI guarda el resultado en el archivo descargable
`E43-player-dependencies`. Opcionalmente acepta `--elf ruta/al/juego.elf
--nm arm-none-eabi-nm` para distinguir símbolos presentes y ausentes.
Presente no significa implementado: puede ser un servicio bloqueado o incompleto.
La revisión se detiene al salir de player.c; no declara completo todo el motor.

Continuar con las dependencias de PlayerInit y PlayerSleep de src/player.c.
No saltar su inicialización para mostrar movimiento con botones.
La compilación de src/player.c a objeto en host ya funciona; sus relocaciones
confirman dependencias en sub_0806FDA0, LoadSwapGFX, InitAnimationForceUpdate,
CheckQueuedAction y la cadena original de items/superficies/animación.

El siguiente bloque gráfico necesita revisar port/port_animation.c (traducción
existente del ASM), transporte nativo de SpritePtr/animaciones y slots de GFX.
gGFXSlots ya tiene almacenamiento en dsi_title_trace.c: no duplicarlo.
PlayerSleep requiere además contexto de script, ExecuteScript y animación;
ExecuteScript sigue bloqueado explícitamente en la ruta E33.
La vista estática actual de Link sólo soporta sus primeros 50 frames: no
presentarla como un renderer completo de despertar/caminar.

Conservar el objeto original 0x4f como Archway; BedCover es 0x5b.
No volver a introducir el sprite magenta que E40 puso en cuarentena.

## Precaución al guardar

El checkout local de esta sesión es una selección de archivos, no un clon
completo del historial remoto. Los commits se publicaron con árboles GitHub
basados en el árbol anterior y actualización del ref con SHA esperado.
No subir ese historial local encima del remoto. Los archivos generated/
son intermediarios; no se versionan.
