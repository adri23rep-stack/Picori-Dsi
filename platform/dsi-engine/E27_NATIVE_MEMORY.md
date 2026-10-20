# E27 — Puente de memoria y datos originales (trabajo en curso)

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
