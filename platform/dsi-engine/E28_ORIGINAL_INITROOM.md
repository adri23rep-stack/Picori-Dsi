# E28 — Portar la inicialización ORIGINAL de habitaciones a Nintendo DSi

## Regla fundamental
NO recrear un motor de habitaciones. El código original está en
`src/gameUtils.c` (`InitRoom()`, `InitRoomTransition()`,
`InitAllRoomResInfo()`, `InitRoomResInfo()`) y `src/game.c`
(`GameMain_InitRoom()`). Los cambios propios de DSi solo deben
adaptar memoria, ROM, gráficos, tablas y periféricos.

## E27 cerrado — pruebas reales
El usuario confirmó en melonDS con ROM EU:
- `Area tables: PASS`
- `Room cache: READY`
- Casa de Link renderizada correctamente.
E27.2 es respaldo. No significa que se ejecute aún el motor jugable.

## E28.0 — Primera prueba realizada
- Prueba automatizada `platform/dsi-engine/tests/run_original_initroom_e28.sh`.
- Extrae **en el momento de compilar** los cuerpos auténticos de
  `InitRoom()` y `InitRoomTransition()` de `src/gameUtils.c`.
- Compila esos cuerpos con tablas `gAreaMetadata` y
  `gLocalFlagBanks` originales (no copias).
- Comprueba la inicialización de flags de área, banco de flags,
  temporizador de portal, iluminación, cambio de transición y
  tratamiento de índices inválidos.
- **Los servicios de `InitAllRoomResInfo` y `SetFade` tienen dobles
  exclusivos para la prueba host**, a fin de aislar sus dependencias
  de hardware. NO se incorporan al ejecutable DSi.
- Las regresiones E25–E27 y la compilación ARM9 del proyecto anterior
  también corren en CI.
- Esto NO habilita todavía `InitRoom()` en la consola.

## E28.1 — Próximos bloqueos reales
1. Analizar `GetAreaRoomHeaderTable()`: Picori PC resuelve las tablas
   a punteros nativos de la ROM completa; DSi solo dispone de ROM
   paginada en SD. Necesita un espejo estable de cabeceras EU.
2. Resolver `InitAllRoomResInfo()`: debe llenar las 64 entradas de
   `gArea.roomResInfos` con datos válidos, sin usar punteros GBA como
   punteros ARM9 ni fingir que todas las habitaciones son la inicial.
3. Adaptar los `MapDataDefinition` originales de la caché E27.2 y
   sus `dest` a RAM GBA sombra, conservando sus campos y durabilidad.
4. Cargar propiedades, salidas y listas originales de entidades sin
   ejecutar punteros de función de la ROM como código ARM.
5. Conectar `InitRoomTransition` con el sistema real de fade de DSi
   manteniendo semántica GBA.
6. Enlazar `src/gameUtils.c` en ARM9 y llamar `InitRoom()` original
   bajo una protección que impida avanzar a `InitUI()`/
   `InitializeEntities()` hasta que sean compatibles.

## Criterio de cierre de E28
- Se ejecuta `InitRoom()` original **en Nintendo DSi/melonDS**,
  con recursos de una habitación real y referencias nativas válidas,
  comprobadas tras cambios de página de caché ROM.
- `gArea.pCurrentRoomInfo`, geometría, flags y punteros esenciales
  son correctos y verificables.
- El escenario que E27 dibujó sigue visible; no se introducen
  duplicados de entidades ni un segundo bucle jugable.
- Prueba host, compilación ARM9 y prueba real pasan.

## Advertencia
Un .nds que solo compila sigue siendo una prueba de compilación.
El primer paso de E28 es un test host del código original; NO es aún
la inicialización completa de habitación en la consola.
