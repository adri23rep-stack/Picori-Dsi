# E14 — Primeras rutinas auténticas de selección de partida (sin escrituras)

## Avance verificable

Esta etapa **compila `src/fileselect.c` ORIGINAL, sin modificaciones**, y ejecuta
únicamente dos de sus rutinas en un entorno controlado tras la transición
real del título a `TASK_FILE_SELECT`:

- `SetFileSelectState(STATE_NONE)`: fija el estado y limpia los mapas auxiliares.
- `ResetSaveFile(0..2)`: prepara tres slots **temporales en RAM** con
  `msg_speed=1`, `brightness=1`, `health=maxHealth=24`.

`DsiFileSelectProbe_Prepare()` solo acepta `gMain.task == TASK_FILE_SELECT`.
El título sigue usando el `ReadKeyInput()` y `TitleTask()` originales.
La pulsación A/START llega a `TASK_FILE_SELECT` (853 ticks de la prueba).
E14 corre las rutinas de archivos después de esa transición. A continuación
se detiene; **NO llama `FileSelectTask()`** porque todavía carecemos de sus
múltiples dependencias reales de renderizado, guardado y entidades.

Para evitar confusiones: **no aparece aún el menú de archivos ni se puede jugar**.
No se crea, modifica, lee ni sobrescribe ningún guardado. No se usa ninguna
ruta de guardados en esta etapa. La ROM `.gba` solo se abre para leer gráficos.
El texto `File select: RAM READY` es un diagnóstico, **no** el menú auténtico.

## Cambios de plataforma

- Se agregaron los chequeos `!defined(TMC_DSI)` en `port/port_ppu.h` y
  `port/port_runtime_config.h` para no depender de SDL3 solo al compilar para
  DSi. El comportamiento del PC sin `TMC_DSI` queda igual.
- `platform/dsi-engine/source/dsi_file_select_probe.{c,h}`: buffer de RAM
  separado de 32 KiB, límite de capacidad comprobado al compilar, puerta
  de seguridad de tarea, comprobaciones de los valores por defecto. Se
  reutiliza el no-op de TTS que el adaptador del título ya proporcionaba.
- `src/fileselect.c` se incluye en el Makefile ARM9 sin introducir
  una pantalla ni lógica alternativa.
- Ajuste menor en el adaptador de gráficos E12 para inicializar dos
  variables antes de que sean rellenadas por el cargador (evita advertencias).

## Pruebas realizadas en Linux con GCC y la ROM EUR del usuario

```sh
sh platform/dsi-engine/tests/run_file_select_e14.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_file_select_e14_sanitized.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_all_e14.sh /ruta/baserom_eu.gba
```

E3–E14: PASS. E14 con AddressSanitizer + UndefinedBehaviorSanitizer: PASS.
También se repitió E12 con sanitizers: PASS. `src/fileselect.c` permanece
sin modificar respecto al archivo E13.

## Compilar en Windows usando devkitPro MSYS

Desde el directorio `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida esperada **si pasa devkitARM**:
`platform/dsi-engine/tmc-dsi-engine-e14.nds`.

Coloca la ROM propia, europea, en la misma ruta FAT/DLDI que antes:
`fat:/tmc-dsi/baserom_eu.gba`. Ejecuta en modo DSi de melonDS. Espera
al logo, pulsa A o START y revisa `File select: RAM READY` en pantalla
inferior. SELECT cierra la prueba.

**La compilación devkitARM y ejecución real de melonDS todavía no se han
probado aquí:** el entorno de trabajo no tiene devkitARM/libnds.

## Siguiente barrera real (E15+)

`FileSelectTask()` real inicializa la pantalla con `ReadSaveFile`, entidades,
UI, fondos y sonido. Antes de llamarlo habrá que implementar una interfaz
segura de **solo lectura** para saves, el mapa real de entidades y el
renderizador de sus fondos/objetos. El objeto original completo de
`fileselect.c` tiene >100 referencias externas distintas (incluyendo
símbolos ya existentes y de funciones que el enlazador puede descartar);
**no equivale a 100 funciones nuevas que falten**. No activar las tareas
que escriben guardados hasta pasar pruebas específicas.

`AgbMain()` sigue sin ejecutarse; E14 llama el título desde un planificador
acotado, no el ciclo general del juego. No hay gameplay todavía.
