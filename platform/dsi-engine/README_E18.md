# E18 — Arranque temporal del juego: frontera real de TASK_GAME

## Objetivo

Dejar la secuencia del título y la selección de partidas **originales** lista
para saltar a la tarea `TASK_GAME`, con datos en RAM y **sin escribir guardados**.
No se implementa un nuevo juego ni se finge que el motor de juego esté corriendo.

## Qué hace

1. E13: `TitleTask()` original recibe START/A e invoca `SetTask(TASK_FILE_SELECT)`.
2. E14–E17: el adaptador prepara tres `SaveFile` temporales llamando a
   `ResetSaveFile()` original y leyendo gráficos/paletas de la ROM EU.
3. E18: en la pantalla de diagnóstico **pulsa X** después de entrar en file
   select. `DsiGameBootstrap_Begin(0)` copia el `SaveFile` seleccionado desde
   la memoria de trabajo a `gSave`, ajusta `gSaveHeader` y llama a **`SetTask(TASK_GAME)` original**.
4. El ejecutable anuncia `Game RAM: READY` y `GameTask: NOT LINKED`. **No**
   llama a `GameTask`, para evitar entrar en código sin dependencias adaptadas.

**Limitación crucial:** `ResetSaveFile()` solo crea valores mínimos (24 cuartos
 de corazón y opciones). Aún no es una creación completa de partida; el
 `saved_status` queda a cero y **no está confirmado como punto de aparición
 válido**. No hay habitación, Link, físicas, audio ni guardado persistente.

## Nuevos archivos

- `source/dsi_game_bootstrap.[ch]`: puente RAM→`TASK_GAME`, sin acceso al disco.
- `tests/engine_e18_game_bootstrap_test.c`: ejercicio de las funciones
  auténticas con la ROM europea, incluye rechazo de tareas y ranuras inválidas.
- `tests/run_game_bootstrap_e18.sh`: compilación de test PC, modo sanitizado.
- `tools/audit_game_e18.sh`: compila `src/game.c` original sin modificar
  y extrae dependencias concretas de sus primeros tres estados.

## Pruebas PC

Desde la raíz de `zelda-tmc-3ds-main`:

```sh
sh platform/dsi-engine/tests/run_all_e17.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tests/run_game_bootstrap_e18.sh /ruta/a/baserom_eu.gba
DSI_E18_SANITIZE=1 sh platform/dsi-engine/tests/run_game_bootstrap_e18.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tools/audit_game_e18.sh
```

El modo sanitizado instrumenta **el adaptador E18 y su test**, no cada módulo
original; no sustituye una validación real de hardware.

## Compilación Nintendo DSi (pendiente de verificar en PC del usuario)

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida prevista: `platform/dsi-engine/tmc-dsi-engine-e18.nds`.
ROM en `fat:/tmc-dsi/baserom_eu.gba` de la SD de melonDS.

**IMPORTANTE**: No está instalado devkitARM en el entorno de pruebas de esta
etapa, por lo que no puedo declarar la compilación ARM9 como aprobada.

## Siguiente objetivo (E19)

`GameTask_Transition()` original depende, de forma directa, de:
`DispReset`, `InitSoundPlayingInfo`, `zMallocInit`, `ResetUI`,
`ClearMenuSavestate`, `MemClear`, `ClearRoomMemory`, `ClearArmosData`,
`FinalizeSave`, `MemCopy`, `ResetTmpFlags` y los datos `gFadeControl`,
`gRoomTransition`, `gSave`, `gMain`.

Prioridades:
1. Resolver `gRoomTransition` y memoria compartida correctamente.
2. Implementar reinicio de salas e inicialización original del juego.
3. Aislar audio y guardado que puedan impedir el arranque, **sin** sustituir
   físicas, entidades ni cargas de habitación por implementaciones inventadas.
4. Conectar `GameTask()` original y ejecutar una transición a `GAMETASK_INIT`.
5. Cargar una habitación real, después el jugador y colisiones.

No confundir `TASK_GAME` seleccionado con un juego iniciado: son hitos distintos.
