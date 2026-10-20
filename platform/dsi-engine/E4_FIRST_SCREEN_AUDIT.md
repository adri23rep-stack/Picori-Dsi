# E4 preparatoria — Primer cuadro auténtico del motor original

## Alcance comprobado

**No se ha iniciado el juego.** La E3 usa funciones reales del motor, pero no
invoca `AgbMain` ni enlaza `TitleTask`. Este informe y la herramienta adjunta
permiten inventariar dependencias sin modificar los algoritmos originales.

En el proyecto original, `src/main.c:AgbMain()` realiza:

1. `InitOverlays`, `InitSound`, `InitDMA`, `InitSaveData`, `InitSaveHeader`;
2. `InitVBlankDMA`, `InitFade`, `MessageInitialize`, `ResetPalettes`;
3. `SetTask(TASK_TITLE)`; y en cada iteración llama al manejador de tarea
   original, `MessageMain`, `FadeMain`, `AudioMain`, `WaitForNextFrame`.

El primer fotograma de título NO es la pantalla con Link. `TitleTask()` comienza
con Nintendo/Capcom, cargando `LoadGfxGroup(16)`, `LoadGfxGroup(1)` y
`LoadPaletteGroup(...)`, y sólo después entra en la presentación del título.

## Dependencias reales inspeccionadas

`src/main.c` y `src/title.c` compilan con un GCC nativo sin tocar sus fuentes.
`nm -u` sobre esos objetos identifica dependencias externas, entre otras:

- Principal: `TitleTask`, `FileSelectTask`, `GameTask`, `GameOverTask`,
  `StaffrollTask`, `DebugTask`, `InitSound`, `InitSaveData`, `InitFade`,
  `MessageInitialize`, `WaitForNextFrame`, `SoftReset` y `RegisterRamReset`.
- Pantalla de título: `FlushSprites`, `CopyOAM`, `LoadGfxGroup`,
  `LoadPaletteGroup`, `SetFade`, `DispReset`, `DrawDirect`, `DrawEntities`,
  `UpdateEntities`, `SoundReq` y estado global de video/entidades.

**La lista es un inventario de símbolos, no una comprobación de ejecución.**
Compilar/enlazar módulos adicionales puede revelar dependencias transitivas.

## Bloqueos estructurales (en orden)

1. **ROM/recursos**: una ROM EU de 16 MiB no puede vivir entera en la RAM de
   DSi. El puente E3 permite leer valores y páginas, pero las rutinas antiguas
   `gRomData + offset` y ciertos punteros C de tablas requieren cambios en
   adaptadores de carga. Evitar entregar punteros volátiles a páginas de caché.
2. **RAM/estado**: declarar/ubicar el resto de globales originales de
   `src/common.c`, `screen`, `save`, tareas y entidades, sin duplicarlos.
3. **Inicialización original**: completar BIOS/GBA/DSI (copia, reseteos, VBlank,
   DMA y timers). El sonido puede quedar temporalmente mudo mediante un
   adaptador, siempre que la lógica original no se modifique.
4. **Renderer real**: `LoadGfxGroup` y `LoadPaletteGroup` deben suministrar
   recursos originales a un renderer GBA->DSI. La E3 todavía no dibuja PPU.
5. **Tarea original**: enlazar `TitleTask` con todos los servicios reales;
   observar el primer logo en pantalla y luego la pantalla de título.

## Primera meta de ejecución verificable

A. Completar servicio de carga de grupo 16 y 1 desde la ROM EU mediante
   búferes de trabajo de tamaño limitado, sin copiar toda la ROM.
B. Verificar paletas/GFX decodificadas y preparar su volcado a las memorias de
   video de DSi con un adaptador, sin redibujar arte ni inventar secuencias.
C. Arrancar la primera tarea original y registrar su estado/ticks en pantalla
   inferior. Sólo decir "primer cuadro logrado" si el motor original carga
   y emite ese cuadro en melonDS.

## Comprobación reproducible

En Linux/WSL con GCC y binutils:

```sh
sh platform/dsi-engine/tools/audit_title_host.sh
sh platform/dsi-engine/tests/run_host_test.sh /ruta/a/ZeldaMinish.gba
```

La compilación real de DSi seguirá siendo:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Este paquete E4 preparatorio **no modifica el binario E3**, no añade ROM y no
pone en funcionamiento `AgbMain`. La E3 debe permanecer operativa.
