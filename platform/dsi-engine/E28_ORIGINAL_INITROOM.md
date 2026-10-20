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

## E28.1–E28.2 — Ejecución ORIGINAL en ARM9 (pendiente validación visual)

- `src/gameUtils.c` y `src/fade.c` originales ya se incluyen en el
  binario ARM9 bajo `TMC_DSI_E28_ORIGINAL_INITROOM`.
- `GameMain_InitRoom()` de `src/game.c` llama realmente a
  `InitRoom()` si E27 tiene una copia válida de los descriptores EU
  y E28 ha fijado las cabeceras EU del área.
- `InitRoom()` original invoca `InitRoomTransition()` original y
  `InitAllRoomResInfo()` original. **El compilador puede optimizar
  `InitRoomResInfo()` e integrarla en su llamador**, por lo que no
  se exige un símbolo separado para esa función.
- El adaptador E28.1 devuelve una tabla persistente de `RoomHeader`
  originales y preserva el terminador `0xFFFF`; no se permite que
  el recorrido de 64 salas salga de la copia nativa.
- Los recursos de mapa, tileset y tiles de la habitación **activa**
  apuntan a copias originales verificadas en RAM. Para otras
  habitaciones se conservan geometría/metadata originales, pero aún
  no se materializan sus flujos.
- `exits`, `properties` y `bg_anim` todavía son `NULL` en el
  adaptador E28; **no significa que el sistema completo de habitaciones
  esté habilitado**. Antes de avanzar a entidades, hay que adaptarlos
  desde datos ROM genuinos y revisar sus consumidores.
- El código original de fade se ejecuta en la transición de juego,
  mientras el diagnóstico de títulos conserva su fade inmóvil; el
  modo iris de GBA no está habilitado en DSi.
- La pantalla inferior reporta
  `Room cache: READY H:OK` y `InitRoom original: PASS` cuando
  la inicialización coincide con el motor y la geometría de la ROM.
  **Esto necesita confirmación en melonDS/DSi con ROM EU.**

GitHub Actions: pruebas host (E25–E28), compilación ARM9 y
comprobación de símbolos `InitRoom`, `InitAllRoomResInfo`,
`InitRoomTransition` y `SetFadeInverted`. Un enlace exitoso
no sustituye la prueba visual en consola.

**No declarado completado todavía:** E28 se cerrará cuando el usuario
confirme la casa visible y ambas comprobaciones nuevas, y se evalúen
las dependencias que todavía se bloquean.

## E28.3 — Seguridad de integración y datos originales de entidades

- `--wrap=InitRoom` instrumenta la llamada a la función **original**; la
  implementación original nunca se sustituye. La estadística
  `original_room_calls` debe ser exactamente 1.
- `DsiGameNative_BootRoomPrefix()` ahora **bloquea el avance** si no existe
  un snapshot EU válido, si no coincide la geometría del `RoomHeader` con
  `DsiRoomResources`, o si la ejecución no llenó correctamente las
  estructuras originales `gArea`/`pCurrentRoomInfo`.
- `dsi_entity_lists.c` copia únicamente los bytes **auténticos** de las
  dos listas de registros EU de 16 bytes que ya verificaba el lector ROM.
  Verifica terminadores `0xFF`, longitudes y límites; conserva el último
  snapshot válido ante un error. **No crea ni actualiza entidades**.
- La pantalla de diagnóstico muestra ahora el contador `init:1`
  de llamadas originales y, junto a la cantidad de píxeles, `E:READY`
  cuando los registros originales se copiaron de la ROM.
- El archivo generado se llama `tmc-dsi-engine-e28.nds`, evitando
  confundir las pruebas con E25.

## E28.4 — Prueba de integración del motor original

La prueba `run_full_initroom_e28.sh` extrae directamente de
`src/gameUtils.c` y compila los cuerpos originales de:

1. `InitRoomTransition()`
2. `InitRoom()`
3. `InitAllRoomResInfo()`
4. `InitRoomResInfo()`
5. `GetCurrentRoomInfo()`

Solo se simulan los límites de ROM/fade y se enlazan las tablas originales
`gAreaMetadata` y `gLocalFlagBanks`. Verifica el centinela de fin de
habitaciones, información correcta de la sala activa, punteros estables a
mapas/tilesets/tiles y falla con seguridad cuando falta la tabla original.

**Criterio para comenzar E29:** primero confirmar en melonDS que la casa
sigue visible y que se muestran `Room cache: READY H:OK`,
`InitRoom original: PASS`, `init:1` y `E:READY`. Estos valores no
prueban todavía NPC ni Link (E29). Antes de ejecutar `InitializeEntities()`
hay que resolver `RoomResInfo.properties`, `exits`, recursos de las demás
habitaciones y los punteros originales a scripts de ROM: actualmente son
`NULL` o identificadores GBA sin convertir; jamás se deben desreferenciar
como punteros ARM9.

E27.2 sigue como respaldo comprobado. No se incluye ROM comercial.

## E28 — VALIDACIÓN EN MELONDS (2026-10-20)

**Hito E28 aceptado.** El usuario aportó una captura del ejecutable
`tmc-dsi-engine-e28.nds` con ROM EU en melonDS. La captura muestra:

- Casa de Link visible, gráficos preservados respecto a E27.
- `Area tables: PASS`
- `Room cache: READY H:OK`
- `InitRoom original: PASS`
- `org:1/1/1 init:1 d:10 e:0` (una llamada original, sin error en
  el diagnóstico del prefijo de arranque).
- `Room pixels:38400 E:READY`
- `Game RAM: READY` y `GameTask: ROOM PREFIX`.

**Qué significa:** el objetivo de E28, ejecutar una primera
inicialización real de habitación mediante `InitRoom()` original y
mantener el renderizado del escenario, quedó demostrado en melonDS.

**Qué NO significa:** el juego todavía no es jugable; la imagen
superior sigue siendo el render de diagnóstico. Los adaptadores dejan
`properties`, `exits`, `bg_anim` en NULL; el contador de
`deferred_services` sigue indicando servicios sin portar. No deben
habilitarse entidades, transiciones completas ni scripts sin atender
esas referencias y sus consumidores.

**Próximo hito E29:** inspeccionar `InitializeEntities()`,
`InitializePlayer()`, estructuras `EntityData`, y dependencias de
propiedades, salidas, listas y sprites reales; conectar progresivamente
las funciones originales sin añadir entidades ficticias.
