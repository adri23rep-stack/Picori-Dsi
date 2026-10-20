# Engine E17 — animación original de paletas del menú (no jugable)

## Cambio verificable

E17 llama a `sub_0805066C()` de `src/fileselect.c` mediante una única función
puente compilada **solo con `TMC_DSI`**. No copia ni reescribe la lógica del
juego: la temporización, selección de paletas y el uso del RNG siguen en el
código original. Para proporcionar a ese código su puntero relativo
`gGlobalGfxAndPalettes`, el adaptador `dsi_file_select_palette.c` copia a RAM
una ventana **0x12000 bytes = 72 KiB** desde la ROM EU que el usuario haya
colocado en la SD. No carga la ROM de 16 MiB ni la incluye en los ZIP.

Esto permite ejecutar de forma segura una subrutina real del menú en la DSi
**después** de que el título haya entrado a `TASK_FILE_SELECT`, sin ejecutar
el controlador completo `FileSelectTask()` ni tocar archivos de guardado.
Solo cambia las paletas en memoria; **todavía no se muestra el menú**.

## Lo que se ha probado aquí

- E3 a E17: pruebas host usando una ROM europea proporcionada para el test.
- La propia rutina del juego actualizó las paletas 11 a 14 en el tick 1 y
  luego en el tick 17. Los 4 bancos de 32 bytes en cada cambio coincidieron
  byte por byte con el archivo original.
- La rutina se niega a avanzar fuera de la tarea de selección de partidas.
- E17 con AddressSanitizer + UndefinedBehaviorSanitizer: pasó para los
  adaptadores probados. Las fuentes originales en este ensayo se compilan
  sin sanitizadores, igual que en E16.
- La ventana de ROM se sirve por páginas de 4096 bytes, manteniendo
  límites explícitos. Sin lectura/escritura de archivos de partidas.

## Lo que NO se ha probado

No está disponible devkitARM en este entorno, por lo que **no se ha compilado
el ejecutable ARM9 ni arrancado E17 en melonDS**. No se asegura funcionamiento
hasta que se pruebe en el PC del usuario. Tampoco ejecuta `AgbMain`, gestiona
un menú real, carga habitaciones ni permite controlar a Link.

## Windows + devkitPro

Desde la raíz `zelda-tmc-3ds-main`:

```
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado esperado si compila: `platform/dsi-engine/tmc-dsi-engine-e17.nds`.
Colocar la ROM europea del usuario en `fat:/tmc-dsi/baserom_eu.gba` en la
SD virtual. Arrancar en modo DSi. Pulsar A o START en la pantalla de título;
si la transición a selección de partida funciona, la pantalla inferior puede
mostrar `Menu pal ticks` aumentando. SELECT sale del programa de prueba.

Para ejecutar las pruebas locales en una máquina con GCC, desde la raíz:

```
sh platform/dsi-engine/tests/run_all_e17.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_file_select_e17_sanitized.sh /ruta/baserom_eu.gba
```

## Archivos cambiados en E17

- `src/fileselect.c`: pequeño puente condicional `TMC_DSI`, no altera la
  implementación original de `sub_0805066C()`.
- `platform/dsi-engine/source/dsi_file_select_palette.[ch]`: adaptador
  de ROM por bloques, transporte DMA host y guardas.
- `platform/dsi-engine/source/boot_dsi.c`: diagnóstico de ticks en DSi.
- `platform/dsi-engine/Makefile`: nuevo objeto y objetivo E17.
- `platform/dsi-engine/tests/engine_e17_file_select_test.c`: verificación
  independiente de los bytes esperados desde la ROM.
- `platform/dsi-engine/tests/run_file_select_e17*.sh` y
  `run_all_e17.sh`: pruebas y regresiones.

## Siguiente bloqueo real

Falta conectar el administrador original de entidades del menú, la creación
de objetos `FILE_SCREEN_OBJECTS`, el cargador de texto y el renderizador, y
luego habilitar cuidadosamente el despacho completo de `FileSelectTask()`.
Las operaciones de guardado seguirán inhabilitadas hasta tener un backend
seguro y probado. El objetivo final sigue siendo portear el motor, no rehacer
la lógica de Zelda.
