# E20 — Primera habitación: validación real de tablas ROM EU

**Alcance comprobado:** lecturas nativas seguras de la información de la primera habitación, usando `DsiGbaBus` y la ROM europea original (BZMP). No se recrea una habitación ni se reescribe su lógica.

## Qué se agregó

- `source/dsi_room_probe.[ch]`: puente de solo lectura que consulta las tablas europeas de `gAreaRoomHeaders`, `gAreaRoomMaps` y `gAreaTable`, con los offsets documentados en `port/port_rom.c` (`kRomOffsets_EU`).
- Lee la entrada original del área `0x22`, habitación `0x15`, y comprueba los rangos de todos los punteros antes de seguirlos. No expone punteros a la caché de la SD.
- Lee las primeras dos listas reales de entidades de la habitación; solo **cuenta registros** hasta su marca final `0xFF`, con límite máximo de 128 registros. **No crea entidades ni ejecuta sus scripts**.
- `boot_dsi.c`: después de START/A y X, verifica la habitación desde la ROM e informa dimensiones y cantidad de registros.
- `tests/run_room_probe_e20.sh`: prueba de PC con ROM EU, más pruebas negativas, memoria y comparación de valores extraídos.

## Resultado con una copia europea válida

| Dato | Valor comprobado |
|---|---|
| Área / habitación | `0x22 / 0x15` |
| Origen en el mapa | `(1296, 432)` |
| Dimensiones | `240 x 160 píxeles` |
| Tileset | `0` |
| Tabla de mapas | `0x08103D50` |
| Tabla de propiedades | `0x080F2854` |
| Listas iniciales de entidades | `3 y 1 registros` |

**Qué NO está hecho:** no se ejecuta `GameMain_InitRoom()` ni `InitRoom()` nativos. Tampoco se cargan los mosaicos del suelo, las colisiones o Link. El motor `src/game.c` se ejecuta solo en la prueba aislada PC E19 con servicios provisionales y defensas; no forma parte del bucle nativo del `.nds`. La lectura de la habitación es una comprobación previa, no la sala jugable.

## Compilar en Windows / devkitPro

Descomprime el proyecto completo en una carpeta nueva. Desde la raíz:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida esperada: `platform/dsi-engine/tmc-dsi-engine-e20.nds`.

La compilación con devkitARM y la ejecución en melonDS **aún deben verificarse en Windows**. En melonDS, SD DLDI habilitada y ROM EU en `fat:/tmc-dsi/baserom_eu.gba` (o el prefijo `sd:/`). Presiona START o A desde el título para llegar a selección de partida y X para iniciar la prueba de una nueva partida en RAM. Debe mostrar:

```
Room ROM: VERIFIED
size: 240x160 area:22 room:15
entity lists: 3 + 1
```

No se modifican archivos de guardado, y el ZIP no incluye ninguna ROM.

## Pruebas PC

```sh
make -C platform/dsi-engine test-e20 ROM=/ruta/a/tu_ROM_EU.gba
make -C platform/dsi-engine test-e20-sanitized ROM=/ruta/a/tu_ROM_EU.gba
make -C platform/dsi-engine test-all-e20 ROM=/ruta/a/tu_ROM_EU.gba
```

El script de prueba de memoria instrumenta el adaptador y el lector de ROM. No sustituye una prueba de hardware DSi.

## Siguiente trabajo bloqueante

1. Identificar el mapa de la primera habitación desde `gAreaRoomMaps` y el tileset desde `gAreaTileSets`; cargar y descomprimir recursos en VRAM sombra, con pruebas byte a byte.
2. Integrar gradualmente las funciones originales `InitRoomResInfo`, `InitRoom` y `LoadRoom`, evitando llamadas a servicios que aún no existan.
3. Enlazar el bucle `GameTask` auténtico en DSi cuando los inicializadores y el administrador de entidades estén disponibles; desactivar temporalmente audio con un servicio explícito y seguro, no con comportamiento de juego nuevo.
4. Habilitar sprites y colisiones originales de Link. Optimizar después de un primer cuarto jugable, sin ignorar corrupción de memoria.
