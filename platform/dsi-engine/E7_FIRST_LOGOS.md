# Engine E7 - Primeros logos autenticos del codigo original

## Que cambiamos

- `dsi_title_trace.c`: `__wrap_DispReset` ahora invoca `ResetScreenRegs()` **original de `src/common.c`**. Este deja `gScreen.bg2.control == 0x1D02` (BG2 usa screenblock 29, charblock 0) y `TitleTask` habilita BG2. Las llamadas completas a DispReset siguen siendo puenteadas, por ahora.
- `dsi_gba_renderer.c`: backend nuevo, separado del juego, que traduce mapa de tiles, datos 4bpp/8bpp, bancos de paletas, flips horizontal/vertical y scroll del estado de GBA al framebuffer RGB555 de DSi. No hay sprites, movimiento inventado ni imagen de logo adjunta.
- `boot_dsi.c`: video de DS modo bitmap 16bpp en la pantalla superior con banco VRAM A, manteniendo consola de diagnostico en pantalla inferior y DLDI. El backend toma `gVram` y `gBgPltt`, cargados desde SD por E6; el BG2 original indica como armar la imagen.
- `run_renderer_e7.sh`: test del renderizador y ROM real, con verificacion de 4981 pixels no-blancos en la ventana GBA, 118 indices de tiles usados y firma FNV1a de la imagen **0x16366349**, calculada sobre 240x160x2 bytes little-endian.

## Estado alcanzado y limites

Confirmado con GCC (PC): `src/title.c` pide los grupos gfx 16 y 1, paleta 2, `ResetScreenRegs` original selecciona BG2 tilemap correcto, y se renderizan los logos Nintendo / Capcom de la ROM EU. La imagen resultante de 256x192 se ha inspeccionado visualmente.

**No confirmado**: build devkitARM ni melonDS, hasta que el usuario lo compile. No se ha realizado ninguna simulacion del juego. Las tres transiciones de TitleTask son una prueba guiada, no el bucle continuo original. No esta conectado `AgbMain()`. Falta recrear en hardware los registros de video mediante backend (sprites/OAM, multiples BG, fades, blend, scanline y modos), audio, save y la logica completa del titulo. Los recursos ROM son leidos por caché, no se carga una copia 16 MB en RAM.

`port_resolve_write_addr` de E7 es un parche limitado para permitir que `ResetScreenRegs()` llame a su propia `MemClear` cuando las direcciones son punteros nativos. **No usar como puente de memoria final**: necesita validacion completa para llamadas con direcciones de GBA.

## Compilar y probar

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
# Pruebas de PC (opcional):
sh platform/dsi-engine/tests/run_renderer_e7.sh /ruta/baserom_eu.gba
```

Archivo `.nds`: `platform/dsi-engine/tmc-dsi-engine-e7.nds`. La ROM no se incluye en el ZIP.

## Proxima etapa

Dejar de ejecutar solo tres llamadas controladas: implementar la sincronizacion de registros BG y composicion de capas de GBA y permitir el avance de `TitleTask` con timers/fades reales del motor. Solo despues comenzar a enlazar la ruta completa de `AgbMain()` con dependencias de audio/guardado.
