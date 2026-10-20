# Zelda: The Minish Cap / Nintendo DSi — E25 (primer cuadro de habitación)

## Objetivo y alcance REAL

Este paquete parte del **E24 que el usuario ya tenía**, no de una recreación del juego. La descompilación `tmc-master.zip` (Project Picori) fue revisada como referencia: su `src/title.c` coincide con el módulo original usado aquí, mientras que integrar `src/game.c`, el estado de ROM y la interfaz PC de Picori completa necesita migración adicional. NO se ha sustituido mágicamente todo el motor por el último Picori.

E25 hace un avance tangible después de E24:

1. Conserva título animado / `TitleTask()` original y las transiciones originales de `GameTask()` (TRANSITION, INIT y prefijo InitRoom) ya integradas por E24.
2. Al pulsar START/A en el título, **intenta continuar automáticamente** a la partida temporal en RAM sin la segunda pulsación de X de E24.
3. Con la **ROM europea original proporcionada por el usuario**, carga dos mapas LZ77 y el tileset de la habitación inicial desde la ROM mediante el puente SD ya existente.
4. Construye y pinta los mosaicos originales de 16x16 (dos capas) con el nuevo módulo `dsi_room_preview.c`. Si el render sale bien, envía un cuadro de la primera habitación a la pantalla superior; el resultado de `ROOM SCREEN: DISPLAYED` aparece en la pantalla inferior.
5. Si algo falla, informa `ROOM SCREEN: BLOCKED` y **no** envía el cuadro incompleto a VRAM. Sin guardar partidas ni necesitar audio.

**NO es jugable.** Todavía no corre `MessageInitialize()`, `InitRoom()`, `InitUI()`, `InitializeEntities()`, ni las tareas jugables posteriores. `Link`, colisiones, físicas y audio NO están portados. Los mosaicos especiales (`0x4000+`) se omiten por ahora. La vista parte de la esquina superior izquierda de la habitación y no incorpora cámara, composiciones avanzadas ni sprites. E25 no asegura que la imagen final sea perfecta.

## Estado de comprobaciones

- ✅ Compilado y ejecutado con GCC de PC el nuevo renderizador y sus pruebas independientes (mosaicos 16x16, paletas, capa superior y límites).
- ✅ Se han enlazado los archivos fuente nuevos en el Makefile DSi y añadido flujo de visualización.
- ⏳ Prueba ROM real europea de E25: pendiente (no se adjuntó `baserom_eu.gba` a esta ejecución).
- ⏳ Compilación ARM9 `.nds` y arranque en melonDS/DSi: pendientes (devkitARM/libnds no están instalados en este entorno).
- ⛔ No se incluye una ROM, ni recursos extraídos ni un `.nds` sin compilar.

## Uso en la DSi / melonDS (tras compilar)

1. Instala **devkitPro** con `devkitARM`, `libnds`, `libfat` y herramienta `ndstool`.
2. En la raíz de este proyecto, ejecuta `make -C platform/dsi-engine clean && make -C platform/dsi-engine` desde devkitPro MSYS (o usa GitHub Actions `Build Zelda Minish Cap DSi E25`).
3. Copia `platform/dsi-engine/tmc-dsi-engine-e25.nds` a tu entorno de DSi o melonDS. Usa modo DSi + SD FAT/DLDI.
4. Coloca tu ROM europea **original sin modificar** en `tmc-dsi/baserom_eu.gba` dentro de la SD (el juego no está en el `.nds`).
5. Abre el `.nds`, espera el título, pulsa START o A, y comprueba la pantalla inferior: `Room prefix: PASS`, `Room tiles: OK` y **`ROOM SCREEN: DISPLAYED`** si la vista se construyó.
6. Si aparece `BLOCKED`, captura la pantalla inferior: la prueba detectó un error. No asumimos que la habitación esté funcionando.

Para probar el render **sin SDK ni ROM**, desde Linux/macOS/WSL:

```sh
sh platform/dsi-engine/tests/run_room_preview_e25.sh
```

Para validar las capas con **tu ROM europea** y guardar una imagen real PNG-convertible (`.ppm`):

```sh
sh platform/dsi-engine/tests/run_room_preview_e25_eu.sh \
  /ruta/a/baserom_eu.gba /ruta/a/room-preview.ppm
```

## Limitación de audio / errores

Sin sonido intencionalmente. No se intenta ejecutar la jugabilidad hasta que podamos resolver punteros ROM y el `InitRoom()` auténtico, porque forzarlo hoy escribiría en direcciones sin mapear. La meta inmediata es mostrar el primer escenario real y luego levantar la lógica, no disimularla con una imagen falsa.
