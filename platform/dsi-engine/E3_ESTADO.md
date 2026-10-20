# Informe E3 — 18 de octubre de 2026

## Objetivo

Portar **código de motor original** de The Minish Cap a DSi, NO recrear el
juego. E3 conserva `src/main.c`, `src/common.c` y ahora `src/flags.c` sin
modificaciones, ejecutando funciones reales en las pruebas de integración.

## Evidencia

Pruebas del host ejecutadas con GCC y ROM EU facilitada:

- PASS `ReadKeyInput`, `SetTask`.
- PASS `src/flags.c` global/local/room + rangos y banderas codificadas.
- PASS ROM paginada y lectura cruzando límites de 4 KiB.
- PASS seguimiento de un puntero GBA dentro de una ROM sintética de prueba,
  lectura de contenido apuntado, validaciones de índice y alineación.
- PASS lectura de la cabecera EU del archivo suministrado.

**devkitARM/melonDS E3 no verificados en este entorno**. La prueba verdadera
la realiza el usuario al compilar y ejecutar `tmc-dsi-engine-e3.nds`.

## Qué falta para arrancar `AgbMain`

Compilé `src/main.c` y `src/title.c` para el host en modo EU para identificar
dependencias: `TitleTask` requiere sistemas de fondo, OAM, entidades,
anima, recursos y audio; `AgbMain` necesita además las funciones BIOS de
GBA, guardado real, pantalla, temporización y tabla de tareas. Compilar un
archivo no significa que sus funciones estén **enlazadas/ejecutadas**.

**Bloqueos por prioridad**:

1. Accesos directos `gRomData + offset`, objetos/tablones C con referencias a
   datos binarios y números de región: cada módulo que pase al núcleo debe
   usar un adaptador de lectura estable o datos residentes específicamente
   dimensionados; NO basta con traducir todos los punteros con un cast.
2. Adaptación de los módulos de inicialización del GBA: `InitOverlays`,
   `InitDMA`, `InitSound`, `InitSaveData`, `InitVBlankDMA`, `InitFade` y
   `MessageInitialize`. Se preserva la lógica y se sustituyen las llamadas
   a hardware GBA/3DS por implementaciones para libnds donde corresponde.
3. Enlazar/renderizar `TitleTask`: los datos y reglas vienen del motor;
   únicamente cambia la salida a las pantallas de DSi.
4. VBlank/timing, DMA real/simulada, audio y acceso fiable a SD.
5. Guardado persistente versionado; pruebas de compatibilidad de región EU.

## Arquitectura de E3

- `dsi_engine_state.c`: posee los globales originales `gSave`, `gArea` y
  `gRoomVars`; no inventa ningún sistema de banderas; ejecuta `flags.c` real.
- `dsi_rom_refs.c`: accede por valor, nunca devuelve puntero a caché SD.
- `dsi_gba_bus.c`: emula un subconjunto de memoria del GBA en RAM de DSi.
- `boot_dsi.c`: invoca funciones originales y muestra resultados verificables.
- Los archivos originales `src/main.c`, `src/common.c`, `src/flags.c` sin
  modificaciones (son agregados al build mediante Makefile).

## Precauciones

No probar E3 escribiendo sobre la única copia de una partida de Zelda: aún
no implementa guardado, pero la ROM se abre en modo `rb` y nunca se modifica.
Si `ROM refs` no pasa, no insistir con `AgbMain`. No distribuir la ROM.

## Próximo hito

E4: integrar inicialización original por tramos con diagnósticos y adaptadores
hardware específicos, enlazar rutas del título. **No prometer pantalla de título
hasta que el motor original la dibuje de verdad**.
