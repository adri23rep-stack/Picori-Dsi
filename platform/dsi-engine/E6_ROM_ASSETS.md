# Engine E6 — TitleTask original: carga real de recursos de ROM europea

## Logrado y comprobado en PC (GCC)

- `src/title.c`, `src/main.c`, `src/common.c` y `src/flags.c` **se conservan sin modificar**.
- Las primeras tres llamadas a `TitleTask()` desde el motor original emiten `LoadGfxGroup(16)`, `LoadGfxGroup(1)` y `LoadPaletteGroup(2)`. **E6 ya no se limita a registrar esas peticiones:** el puente las lee de las tablas originales de la ROM europea y realiza sus copias hacia el espacio de memoria virtual del GBA.
- Las tablas de punteros se ubican en las direcciones EU documentadas por `kRomOffsets_EU` en `port/port_rom.c`: `gfxGroups=0x100204`, `paletteGroups=0x0FED88`, `gfxAndPalettes=0x5A23D0`.
- Los grupos iniciales cargan **4 registros gráficos (9,248 bytes)** y **2 registros de paleta (96 bytes)** en direcciones de GBA (video y paleta). El código comprueba número, límites, valores y fin de registro.
- Incluye descompresión LZ77 (tipo 0x10 de GBA) a RAM/video virtual con comprobaciones de tamaño, distancia y escritura válida. Prueba sintética independiente para bloques comprimidos.
- La prueba de ROM real verifica **byte por byte** las cuatro transferencias gráficas y las dos transferencias de paleta contra la ROM EU original del usuario. No empaquetamos ni extraemos gráficos o ROM en los ZIP.
- Los tests anteriores de E3 y E5 siguen pasando.

## No se ha logrado todavía

- **No hay logos visibles**: se ha cargado memoria gráfica virtual, pero aún no existe la traducción de tilemaps, capas, registros y paletas a la pantalla física de DSi.
- `AgbMain()` sigue sin ejecutarse. Audio, guardado y gráficos reales del motor completo aún faltan.
- Solo están implementados los grupos requeridos por estas primeras llamadas. No afirmamos haber validado todos los recursos o combinaciones de idioma y región.
- El wrapper de `LoadGfxGroup`/`LoadPaletteGroup` sigue activo; invoca el nuevo cargador. `DispReset` y `MemClear` siguen envueltos, y las transiciones dependen del puente diagnóstico. Esto **NO significa que el motor esté corriendo a velocidad real ni que la pantalla de título esté lista**.
- No tenemos devkitARM ni melonDS en este entorno para comprobar la compilación real de `.nds`. El usuario debe confirmar el resultado en Windows.

## Compilación en Windows, devkitPro MSYS

Desde la carpeta `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Archivo esperado: `platform/dsi-engine/tmc-dsi-engine-e6.nds`.

ROM europea propia: `Sd Melon/tmc-dsi/baserom_eu.gba` (igual que E5).

Pantalla inferior esperada si funciona: `TitleTask: PASS (3 steps)`, `ROM assets: PASS`, `Asset errors: 0`, `GFX bytes: 9248`, `PAL bytes: 96`, `Renderer: NOT LINKED`. La ejecución no ha sido comprobada en melonDS todavía.

## Tests para PC (GCC, fuera de melonDS)

```sh
sh platform/dsi-engine/tests/run_host_test.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e5.sh
sh platform/dsi-engine/tests/run_title_e6.sh /ruta/a/baserom_eu.gba
```

`make -C platform/dsi-engine test-assets ROM=/ruta/a/baserom_eu.gba` también ejecuta el último test si hay GCC y Python 3 en el entorno.

## Siguiente objetivo

Integrar `DispReset`, `ResetScreenRegs` y actualizaciones del sistema de video del motor con un backend que sincronice la memoria virtual del GBA a las capas gráficas de DS. Luego intentar dibujar el primer logo **a partir del estado que produjo `TitleTask()`**, no mediante una imagen extraída de la ROM ni una escena dibujada a mano.
