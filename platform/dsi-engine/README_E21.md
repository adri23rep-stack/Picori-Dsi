# E21 — Carga original de las capas del mapa de la habitación de Link

**Objetivo:** seguir la ruta corta hacia una partida, no reconstruir el juego.

Partiendo de los punteros comprobados en E20, `dsi_room_maps.c` interpreta la tabla **original** `MapDataDefinition` de 12 bytes de la ROM europea. Es la adaptación de transferencia del camino de ROM de `LoadMapData()` en `src/beanstalkSubtask.c`: lee el desplazamiento respecto de `gMapData` (EU `0x08323FEC`), la dirección de destino de la RAM del GBA y el tamaño/indicador LZ77. La operación va a **RAM virtual del GBA**, no directamente al hardware DSi.

Para la primera habitación (área `0x22`, habitación `0x15`) se comprobaron **dos bloques LZ77 de 300 bytes**, en las direcciones originales `0x02025EB4` y `0x0200B654`. La versión E21 comprueba el tamaño descomprimido esperado, el límite de cada bloque, la longitud de la cadena, los destinos disponibles y el formato de compresión. No inserta mapas o físicas inventadas.

Las salidas descomprimidas coinciden con un decodificador de referencia independiente de los adaptadores: FNV-1a inferior `EB1EF36B`, superior `6B637C85`. Son comprobaciones de una ROM europea de 16 MiB. Esos códigos son verificadores **no criptográficos** de los 300 bytes, no identificadores de la ROM entera.

Para compilar en Windows con devkitPro, desde la raíz del proyecto:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

La salida esperada es `platform/dsi-engine/tmc-dsi-engine-e21.nds`. En melonDS, con la SD configurada y `baserom_eu.gba` en `tmc-dsi`, pulsa START o A, después X. La pantalla inferior debe mostrar `Room maps: OK 600 B`. Eso confirma **datos del mapa en RAM virtual**, NO que el mapa se haya dibujado ni que el motor `GameTask()` esté ejecutándose de manera nativa en DSi.

Pruebas de computadora:

```sh
make -C platform/dsi-engine test-e21 ROM=/ruta/a/baserom_eu.gba
make -C platform/dsi-engine test-e21-sanitized ROM=/ruta/a/baserom_eu.gba
make -C platform/dsi-engine test-all-e21 ROM=/ruta/a/baserom_eu.gba
```

El test comprueba longitud, doble descompresión, contenido original, límites y que una referencia inválida no cambie la RAM. **Pendiente:** prueba ARM9/devkitPro/melonDS; todavía falta cargar el tileset, reconstruir la composición usando funciones originales de tiles, integrar la inicialización del mundo y, después, el jugador y sus colisiones.

No se empaqueta la ROM ni se modifican guardados. Se mantienen los módulos originales del motor sin modificar.
