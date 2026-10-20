# Engine E24 — primer tramo de GameMain_InitRoom (port nativo parcial)

## Qué hace realmente

- Al pulsar START o A en el título se llega a la tarea original de selección de partida.
- Al pulsar X se crea una partida TEMPORAL en RAM y se llama a las funciones originales `GameTask_Transition` y `GameTask_Init`.
- Se validan las referencias de la habitación de Link en la ROM europea y se cargan sus mapas y mosaicos con los adaptadores anteriores.
- **Nuevo en E24:** con un permiso de uso único, una tercera llamada a `GameTask()` ejecuta el inicio de `GameMain_InitRoom` de `src/game.c`. Se ejecutan la llamada a `SetInitializationPriority` (adaptador equivalente a la asignación original), la preparación de `displayControl`, el cambio al subestado `GAMEMAIN_CHANGEROOM` y el reinicio de campos de transición. Las condiciones se verifican.
- El compilador DSi utiliza `-DTMC_DSI_SAFE_GAME_BOOT`: ese guardado solo existe para esta compilación; en los demás builds sigue el flujo original completo.
- El programa **se detiene antes de `MessageInitialize()`, `InitRoom()`, `InitUI()` e `InitializeEntities()`**. Esto es intencional. No hay habitación mostrada ni lógica jugable aún.
- Las 10 funciones de servicios diferidas de E23 siguen diferidas. No se oculta este dato.
- El ajuste de diagnóstico E23.1 está incluido: los mensajes están en filas fijas, sin scroll.

## Cómo compilar (Windows / devkitPro MSYS)

Desde `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado esperado: `platform/dsi-engine/tmc-dsi-engine-e24.nds`.

En melonDS DSi: START o A, luego X. Se espera ver `Room prefix: PASS`, `GameTask: ROOM PREFIX`, `orig:1+1+1`, `Room tiles: OK`. La pantalla superior seguirá mostrando el último cuadro del título.

## Pruebas host

```sh
make -C platform/dsi-engine test-e24 ROM=/ruta/a/baserom_eu.gba
make -C platform/dsi-engine test-e24-sanitized ROM=/ruta/a/baserom_eu.gba
```

La ROM original NO va en estos ZIP. Los tests de PC no sustituyen la compilación ni la ejecución en DSi real.

## Siguiente bloqueo: `InitRoom()`

Necesitamos portar la resolución de tablas/punteros de área y habitación al backend de ROM por páginas y asignar en RAM las estructuras `gArea`, `RoomResInfo`, listas de entidades, mapas y colisiones antes de llamar a `InitRoom` entero. También integrar las funciones de guardado, paletas y entidades que el inicio de habitación utiliza. No conviene ejecutar `InitRoom()` directo con punteros de GBA sin traducir: podría escribir en memoria errónea.
