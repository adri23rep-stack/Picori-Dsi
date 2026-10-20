# E27 — Puente de memoria y datos originales (E27.2 compilando)

## Regla
Portar el **motor original** de The Minish Cap. Solo construir adaptadores
del hardware GBA a Nintendo DSi, nunca recrear movimiento, enemigos o física.

## Cambios E27.1
- El ARM9 enlaza el archivo original `src/data/areaMetadata.c`:
  `gAreaMetadata` y la tabla completa `gOverworldLocations`.
  Se eliminó el dummy de una única ubicación que no podría alimentar al
  `UpdateWindcrests()` original.
- Nuevos `DsiRomRefs_ReadRomPointer()` y
  `DsiRomRefs_RomArrayEntry()` leen direcciones **GBA de 32 bits**
  almacenadas en tablas de la ROM europea. Nunca devuelven un puntero
  de C a memoria virtual GBA. Rechazan desbordamientos, direcciones de
  hardware RAM/IO, desalineación, punteros inválidos y ROM truncada.
- El lector de datos de habitación `dsi_room_probe.c` y el cargador de
  tiles `dsi_room_tiles.c` usan los lectores verificados.
- Tests host sin ROM comercial cubren tabla de áreas original y punteros
  sintéticos, buenos y corruptos. GitHub Actions genera el .nds.

## Auditoría de dependencia: ruta auténtica hasta InitRoom()
1. `GameMain_InitRoom()` de `src/game.c` llama las funciones originales
   `MessageInitialize()`, `InitRoom()`, `InitUI()`,
   `InitializeEntities()`.
2. `InitRoom()` en `src/gameUtils.c` limpia `gArea`, usa
   `gAreaMetadata[area]`, `gLocalFlagBanks[flag_bank]` y configura
   tiempo/luz; luego llama a `InitRoomTransition()` y
   `InitAllRoomResInfo()`.
3. `InitAllRoomResInfo()` consume los punteros de `RoomHeader`, los
   descriptores de tiles/mapas, propiedades, salidas de habitación y
   `RoomResInfo`. Muchos viven en la ROM como direcciones GBA
   `0x08xxxxxx` y necesitan adaptación o almacenamiento nativo.
4. `InitRoomTransition()` lee estado del jugador, sala y transiciones
   de guardado. Se debe auditar antes de desbloquear la llamada.
5. `src/data/areaMetadata.c` en Picori utiliza tablas base USA con
   ciertas adaptaciones EU. Falta comprobar paridad EU en hardware.

## Criterios E27
- [x] Variables de área/ubicaciones originales integradas en ARM9.
- [x] Resolución de punteros de ROM limitada y probada con fixtures.
- [ ] Confirmar nuevo binario en melonDS/DSi y su carga EU.
- [ ] E27.2: diseñar y probar almacenamiento persistente de las
  `RoomResInfo` y tablas originalmente apuntadas a ROM.
- [ ] Conectar `gArea` y las estructuras/servicios reales antes de
  ejecutar `InitRoom()`.

**E27 no activa InitRoom() ni ejecución de Link.** Una compilación
exitosa no es prueba de jugabilidad. E25 y E26 están preservadas.


## E27.2 — Memoria nativa persistente (2026-10-20)

- `platform/dsi-engine/source/dsi_room_resources.c` copia **los registros
  originales de 12 bytes** de `MapDataDefinition` de mapa, tileset y tiles
  directamente desde los punteros EU de la ROM. Conserva `src/dest/size`
  empaquetados en una estructura estable de RAM ARM9; **no** intenta
  reinterpretar direcciones `0x08...` como punteros nativos.
- Los tres flujos guardan hasta 16 registros cada uno, exigen fin de
  cadena y comprueban destinos GBA, paletas y tamaños. Si hay error,
  conserva intacta la última copia válida.
- `DsiRoomResources_PrepareEU()` prepara esos datos junto a geometría,
  índices, direcciones de listas de entidades y conteos de la habitación.
  `DsiRoomResources_Current()` devuelve una dirección **de RAM nativa**
  estable hasta reset o preparación exitosa siguiente.
- `Area gArea` y `RoomVars gRoomVars` **ya estaban presentes** en `dsi_engine_state.c` desde etapas anteriores. E27.2 reutiliza estas estructuras originales en ARM9 y no crea estados paralelos.
- En la pantalla inferior el arranque muestra `Room cache: READY`
  cuando consigue verificar y fijar los tres flujos de datos originales.
  Este paso no altera el renderizador E25, no ejecuta `InitRoom()` y
  no inventa un motor paralelo.
- Nueva prueba de host con una ROM sintética verifica flujo encadenado,
  comando de paleta, vida útil, datos inválidos, atomicidad y reset.

**Aceptación de E27.2:** compilar en Actions y probar en melonDS/DSi con
la ROM EU del usuario: `Area tables: PASS`, `Room cache: READY`,
y la casa visible como antes. Si hay `PENDING` resolver diferencias de
los descriptores, sin saltar a E28.

**Siguiente paso E28:** adaptar `RoomResInfo` original a punteros
nativos válidos y conectar `InitRoom()` auténtico de `src/gameUtils.c`,
`InitRoomTransition()`, `InitAllRoomResInfo()` y su estado asociado,
sin desactivar validaciones de memoria.
