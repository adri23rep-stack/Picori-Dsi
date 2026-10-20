# Engine E5 — Ejecución de `TitleTask()` original (diagnóstico)

## Qué está probado

Esta versión **enlaza y llama** a `TitleTask()` en `src/title.c` SIN
modificar el archivo original. También enlaza `src/main.c`, `src/common.c` y
`src/flags.c`. El test de PC ejecuta tres llamadas a `TitleTask()`:

1. `SetTask(TASK_TITLE)` -> la tarea inicializa mensajes y comienza la
   transición del logo.
2. Al terminar de forma *simulada* el efecto de fundido, la tarea pasa al
   estado que despacha el logo.
3. La tarea solicita los grupos gráficos **16** y **1** y la paleta **2**
   (ROM europea, idioma inglés). Estas solicitudes se registran.

Se comprueban los estados originales, las solicitudes y los números de llamadas.
También se comprueba que los servicios no implementados no sean invocados en
estos tres pasos. No se copian ni dibujan gráficos; el fundido es un
*adaptador de prueba* sin temporización real.

**Importante:** `LoadGfxGroup`, `LoadPaletteGroup`, `DispReset` y `MemClear`
son interceptados por el enlazador (opción `--wrap`) durante E5. Las funciones
originales siguen en el código fuente, pero estas cuatro llamadas del
experimento no ejecutan la implementación original. El puente documenta las
solicitudes sin fingir que los datos gráficos se hayan cargado. Cuando
conectemos el cargador de recursos retiraremos esas intercepciones una por una.

## Qué NO está funcionando

- `AgbMain()` todavía **no se ejecuta**.
- No se renderiza el logo ni la pantalla de título.
- Audio, fundidos reales, guardado, sprites y transición al menú siguen pendientes.
- El código de E5 todavía **no está compilado ni ejecutado en hardware DSi**
  desde este entorno; la primera compilación será en tu devkitPro y la primera
  ejecución en melonDS.

## Prueba en PC (realizada)

```sh
sh platform/dsi-engine/tests/run_host_test.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e5.sh
```

## Compilación en Windows con devkitPro MSYS

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado **esperado**, no garantizado hasta compilarlo:
`platform/dsi-engine/tmc-dsi-engine-e5.nds`.

Debería mostrarse `TitleTask: PASS (3 steps)` y
`GFX requests: 16,1 PAL2` con ROM europea presente en
`fat:/tmc-dsi/baserom_eu.gba`.

## Siguiente avance prioritario

Implementar `LoadGfxGroup(16)` / `(1)` y `LoadPaletteGroup(2)` contra la ROM
EU, con tablas de offsets válidas para esa región, descompresión GBA (incluido
LZ77), copias seguras a búferes de GBA y transferencia al video de DSi. Esto
sustituirá las intercepciones de registro por cargas reales.
