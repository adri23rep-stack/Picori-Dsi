# Engine E22 — Tileset y datos originales de la habitación de Link

**Objetivo:** ampliar E21 hasta cargar los recursos necesarios antes de reconstruir la imagen de una habitación. No reimplementa `LoadRoomTileSet()`, `LoadRoomGfx()` ni la jugabilidad; adapta los registros `MapDataDefinition` de la ROM europea para llenar la memoria virtual del GBA de forma acotada.

## Datos comprobados

- Área **0x22**, habitación **0x15**, tileset **0**.
- E21: 2 capas de índices de mapa LZ77, **600 bytes** en RAM virtual.
- E22: cadena **gAreaTileSets[0x22][0]** de la ROM europea: tres transferencias LZ77 a VRAM virtual GBA, **49,152 bytes**, y el comando original de paleta **grupo 0x34**.
- E22: cadena **gAreaTiles[0x22]** de la ROM europea: cuatro transferencias LZ77 a EWRAM virtual, **36,980 bytes**.
- E22 en total: **86,132 bytes nuevos**, siete bloques LZ77 más la petición de paleta original.
- Los siete bloques fueron comparados usando FNV-1a de los datos descomprimidos, con valores obtenidos por un descodificador Python independiente (ver `tests/engine_e22_room_tiles_test.c`).

El código nuevo está en `source/dsi_room_tiles.c`. Reutiliza el puente `source/dsi_room_maps.c` de la etapa anterior y le incorpora la orden original `dest == 0` para cargar un grupo de paletas. **No inventa una definición de tiles o un nuevo sistema de colisiones**.

## Compilación DSi (pendiente de comprobar)

Desde la raíz del proyecto, en devkitPro MSYS:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida esperada: `platform/dsi-engine/tmc-dsi-engine-e22.nds`.

La ROM europea del usuario permanece en la SD virtual: `sd:/tmc-dsi/baserom_eu.gba` o equivalente `fat:/...`. Al abrir el título, pulsar START o A, luego X, el diagnóstico intentará cargar mapas, gráficos de tileset y definiciones originales. Busca `Room tiles: OK 86132 B`. La pantalla de juego aún no es visible ni se ejecuta `GameMain_InitRoom()` en la DSi.

## Pruebas en PC

```sh
sh platform/dsi-engine/tests/run_room_tiles_e22.sh /ruta/a/ZeldaMinish.gba
DSI_E22_SANITIZE=1 sh platform/dsi-engine/tests/run_room_tiles_e22.sh /ruta/a/ZeldaMinish.gba
```

También pasaron por separado las pruebas normales y sanitizadas de E20 y E21. La suite histórica E3–E19 alcanzó `PASS` y la ejecución total se cortó después por límite de tiempo; **no se debe confundir con una verificación ARM9 ni con E3–E22 end-to-end**.

## Lo que sigue

1. Enlazar y ejecutar en la DSi las rutinas originales `LoadRoomTileSet()`, `LoadRoomGfx()` y construcción de subtile maps con sus dependencias reales (ver `src/playerUtils.c` y `src/beanstalkSubtask.c`).
2. Resolver, sin punteros de ROM persistentes, los recursos de paleta y las reglas de mapa que todavía faltan; presentar el primer cuadro construido por el motor original.
3. Conectar `GameMain_InitRoom()`, inicialización de entidades, jugador y colisiones originales.

**Alcance**: E22 todavía es un port parcial de carga de recursos, no una partida jugable. No escribe guardados y no contiene ROM ni recursos comerciales empaquetados.
