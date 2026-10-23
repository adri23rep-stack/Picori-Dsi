# E47 — Continuación de la rama nueva de Astra

## Base verificada
Se parte **directamente** de la rama `e41-original-interior-collision` de Astra,
commit `7d2ca0b0d0b10eb3077c27b3b27b716ea24fd2dc` (E46), no de nuestro
experimento E50 ni de E54. **No se sobrescribe el trabajo de Astra.**

Astra aportó en E44–E46:
- el gestor original de GFX slots y sprite extra `sub_0806FDA0`;
- carga desde la ROM europea de flujos de animación, fotogramas y tiles
  originales de Link, incluidos dormir y despertar;
- `InitAnimationForceUpdate`, `UpdateAnimationSingleFrame` y el
  transportador de gráficos a VRAM sombreada GBA.
- pruebas con miles de ticks de animación contra la ROM EU.
  La sincronización correcta de `ANIM_DONE` para despertar sigue sin
  integrarse en `PlayerSleep`.

## Nuevo E47
Se incorporaron sin alterar las funciones fuente desde la línea experimental E50:
- las consultas GBA `GetActTileAtTilePos`, `GetActTileAtEntity`,
  `GetActTileRelativeToEntity`, `GetTileTypeRelativeToEntity`,
  `GetTileIndex`, `GetLayerByIndex`, `FindValueForKey`;
- los helpers originales `GetMapLayerForLayer`, `WorldToTilePos`,
  `NormalizeTilePos`, y el detector de superficie `PlayerCheckNEastTile`;
- el cálculo original de tile frente al hitbox `sub_0806F730`;
- tabla original de datos `mapActTileToSurfaceType.c`, compilada intacta.

En `boot_dsi.c`, solamente después de la preparación correcta de
colisiones E41/E42 y entidad de jugador, se consulta el tile real de Link
con `PlayerCheckNEastTile()`, se imprime `E47 terrain: ORIGINAL`
con el resultado 0/1 y se continúa la ejecución de diagnóstico normal.

Se comprueba la identidad literal de cada función extraída con las fuentes
`port/port_linked_stubs.c`, `port/port_gameplay_stubs.c`,
`src/physics.c` y `src/scroll.c`. No se recrea el sistema de colisiones.

## Lo que **NO** está implementado
- No se ejecuta `PlayerInit` original ni el despachador de acciones
  `DoPlayerAction` desde un bucle de juego.
- No se actualizan los fotogramas de Link de forma activa en la habitación;
  el preview E35 es estático. No hay control jugable.
- No se procesa el script original de despertar, transición de escenas,
  NPCs, combate ni persistencia de guardados.
- La animación y la carga de tiles en GBA shadow existen pero falta
  el render OAM completo de Link y el transporte a hardware DSi real.

## Pruebas y compilación
CI: https://github.com/adri23rep-stack/Picori-Dsi/actions/workflows/dsi-e41.yml
Rama: `e47-original-terrain-integration`
NDS: GitHub Actions artifact `Picori-DSi-E47-NDS` al pasar host y ARM9.

Para melonDS, habilitar DSi + SD, cargar ROM EU personal como
`/tmc-dsi/baserom_eu.gba`, abrir `Picori_DSi_E47.nds`.
Esperar `E47 terrain: ORIGINAL 0` o `ORIGINAL 1` (dato original),
o `BLOCKED` si la preparación falla; confirmar que Link sigue dormido
en la habitación y que no aparezcan cuadros corruptos.

**No afirmar 'jugable' por una compilación exitosa**. E47 conserva el
proyecto y el comportamiento base del E46 de Astra.
