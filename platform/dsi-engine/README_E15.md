# Nintendo DSi Engine E15 — Recursos originales de selección de partida

## Resultado real

Desde la transición auténtica de `TitleTask()` a `TASK_FILE_SELECT` se ejecutan **las funciones originales**:

- `SetFileSelectState(0)` y `ResetSaveFile(0..2)` (E14): preparar tres ranuras en RAM sin leer ni escribir ningún archivo de guardado.
- `LoadOptionsFromSave(0..2, 3)` en `src/fileselect.c`: copia los valores originales de brillo y velocidad de texto y marca las paletas como usadas.
- `sub_080503A8(5)` en `src/fileselect.c`: solicita el grupo de gráficos 5, exactamente el que usa el `HandleFileScreenEnter()` original.
- El adaptador pide la paleta 9, como en `HandleFileScreenEnter()`, mediante el cargador de ROM E6 (`__wrap_LoadPaletteGroup`).

Los datos se cargan a memoria virtual del GBA, nunca se inyectan en archivos de guardado; después el programa **se detiene intencionalmente antes de llamar `FileSelectTask()`**. No existe todavía la interfaz gráfica del menú de partidas.

Las pruebas nuevas verifican que los primeros 64 bytes de VRAM y de paletas cargados coinciden **byte por byte** con los registros de las tablas de la ROM europea, decodificadas de forma independiente del cargador.

## Seguridad de partidas

- Este port solo abre `baserom_eu.gba` para lectura.
- Ningún archivo `.sav` se lee, crea, escribe o borra.
- `SetActiveSave`, `ReadSaveFile`, `WriteSaveFile` y `FileSelectTask` **no se ejecutan**.
- Las tres ranuras están en un buffer RAM separado de 32 KiB.
- `LoadIntroAssets` y `TestOptions` rechazan tareas distintas de `TASK_FILE_SELECT` y ranuras sin preparar.

## Compilación en Windows con devkitPro

Desde `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida esperada **si devkitARM compila correctamente**: `platform/dsi-engine/tmc-dsi-engine-e15.nds`.
No se ha ejecutado compilador devkitARM en el contenedor de trabajo; el `.nds` y melonDS están pendientes de tu comprobación.

La ROM europea debe seguir en `fat:/tmc-dsi/baserom_eu.gba` dentro de tu SD virtual DLDI. El título sigue esperando A/START; en la pantalla inferior deberá salir `File-select: ASSETS OK` y los contadores de bytes de gráficos/paletas. No se dibuja un menú falso encima del título.

## Pruebas de PC

Con una copia propia de la ROM EUR:

```sh
make -C platform/dsi-engine test-all-e15 ROM=/ruta/baserom_eu.gba
make -C platform/dsi-engine test-e15-sanitized ROM=/ruta/baserom_eu.gba
```

Los módulos originales `src/main.c`, `src/title.c`, `src/fileselect.c` y `src/common.c` no se modificaron; los cambios son solo en `platform/dsi-engine/`.

## Pendientes críticos

1. **Arranque real** de `FileSelectTask()`: necesita UI, objetos, texto, guardado y sonido reales.
2. Renderizar mapas/paletas originales del menú desde el estado exacto del motor, no un menú redibujado a mano.
3. Soporte de lectura de guardados con garantías y pruebas antes de activar cualquier escritura.
4. `AgbMain()` sigue sin enlazar/ejecutar y falta el motor de habitaciones, físicos y combate.
5. Validación de memoria/rendimiento en DSi real y melonDS; las pruebas de PC no equivalen a un arranque nativo comprobado.
