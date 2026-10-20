# Informe tecnico E2 (18 octubre 2026)

## Objetivo acordado
Portear de verdad el motor reconstruido 3DS/PC/GBA a Nintendo DSi. **No**
reescribir el movimiento de Link ni sustituir el motor con una demo.

## Cambios realizados
1. `source/dsi_gba_bus.c/.h`: nuevo puente de memoria GBA nativo DSi,
   direccionamiento 8/16/32 bits, EWRAM 256 KiB, IWRAM 32 KiB,
   memoria de video GBA 96 KiB, I/O, paletas y OAM. Con proteccion de limites.
2. Reutilizacion del lector de ROM existente `tmc_rom_stream.c` (8 paginas x
   4 KiB), con direccionamiento GBA 0x08000000 y copia ROM -> RAM.
3. `source/boot_dsi.c`: pantalla E2 que llama **realmente** a
   `ReadKeyInput()` en `src/common.c` y `SetTask()` en `src/main.c`.
4. `Makefile`: anade el puente, genera `tmc-dsi-engine-e2.nds`.
5. `tests/engine_e2_test.c`: pruebas de funciones originales, bus,
   lecturas ROM y acceso a pagina cruzada (usa ROM temporal sintetica).
6. `tests/run_host_test.sh`: compila los modulos originales y puente;
   permite opcionalmente probar ROM propiedad del usuario.

## Resultados verificables
- GCC host + prueba EU provista: PASS (ReadKeyInput, SetTask, bus,
  ROM cruzando pagina, copia ROM -> RAM y deteccion de region).
- ASan+UBSan sobre puente y pruebas, enlazado con motor sin instrumentar:
  PASS. Instrumentar todo el motor manteniendo codigo descartado por
  `--gc-sections` introduce dependencias de funciones no enlazadas.
- Compilacion devkitARM en Windows / arranque melonDS E2: NO PROBADOS AQUI.
- `AgbMain()` y `TitleTask`: NO ejecutados, NO integrados funcionalmente.

## Restricciones importantes
El lector de ROM valida solo la cabecera del codigo de juego BZMP/BZME,
**no** SHA1. E2 no reproduce todavia las aperturas de ROM que realizan las
funciones de juego usando `gRomData` o punteros constantes a 0x08xxxxxx.
Tampoco realiza mapeo de GBA ROM espejo, reflejos de RAM, temporizadores,
VBlank, DMA o IRQ originales. Es un puente parcial, no una emulacion
completa de GBA.

## Integridad del proyecto
- `src/main.c` y `src/common.c` no fueron modificados.
- `platform/dsi` (demo V4) no fue modificado.
- Ningun archivo `.gba` ni imagen extraida de ROM entra en los ZIP.
- El objetivo de E2 es reducir dependencias del motor original, no fabricar
  jugabilidad que no existe.
