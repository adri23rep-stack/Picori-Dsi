# E10 — Integración de comandos OBJ reales de la pantalla de título (EU)

## Qué se integró

- La función original `TitleTask()` de `src/title.c` sigue ejecutándose intacta.
- `DrawDirect(sprite,frame)` ya no es un contador vacío: pasa los comandos de
  `gOamCmd` al adaptador `dsi_title_sprites.c`.
- Este adaptador lee **los registros reales de `gFrameObjLists` de ROM EUR** por el
  bus paginado (ROM `0x082F3460`, longitud `199561`), y la tabla de tamaños/anclas
  del ROM (`0x080B2310`). Solo se guardan cuatro frames frecuentes en una caché.
- Convierte las piezas originales (cinco bytes por pieza) a atributos de OAM de
  GBA y permite un compuesto de píxeles OBJ 4/8bpp normal, no afín, usando la VRAM
  y las paletas sombra. No implementa reglas o movimiento del juego.
- Se detectan los cambios de piezas OBJ en `DsiTitleLive_Render`, para redibujar
  cuando cambie el conjunto visible sin volver a dibujar innecesariamente.
- `TitleTask()` avanza hasta un límite seguro de 800 ticks adicionales; no pasa
  automáticamente a file-select ni gameplay.

## Pruebas hechas en este entorno de PC con ROM EUR del usuario

- Todos los tests heredados E3 a E9 pasaron.
- E10: 560 ticks adicionales después de E9, **427 `DrawDirect` auténticos**,
  **1564 piezas** contabilizadas a lo largo de esos cuadros, **10 cambios del
  conjunto de OBJ**, **0 errores de descodificación**.
- La caché de frame OBJ tuvo **425 aciertos y 2 fallos** para esas llamadas.
- Se comprobó un frame **original** (índice 510, pose 1) con cuatro piezas.
- Se probó el motor de píxeles OBJ con un píxel rojo **artificial** inyectado en
  VRAM y paleta sombra; el pixel de salida fue `0x801F`. NO es un gráfico del
  juego ni una captura de un objeto real en melonDS.
- Una prueba de sprite inexistente rechazó el índice 511, sin leer fuera de
  la región de ROM.
- Test E10 con AddressSanitizer + UndefinedBehaviorSanitizer: ver salida del
  script `tests/run_title_e10_sanitized.sh`.

## MUY IMPORTANTE: qué NO se ha logrado

- Los sprites de título **NO son visibles todavía**: la rutina original de
  carga de gráficos OBJ por `CreateObject` y subsistemas asociados NO está
  conectada. El demo E10 sí tiene un renderer OBJ, pero carece de los tiles
  que deberían llenar sus referencias de OAM durante el título.
- `CreateObject`, `UpdateEntities`, `SoundReq`, fades completos y otras
  dependencias siguen instrumentadas. `src/main.c` está parcialmente enlazado,
  pero `AgbMain()` todavía NO corre.
- El compuesto OBJ actual admite solo objetos normales, sin modos afines ni
  objetos ventana/semiprueba ni prioridad de BG compleja; puede dibujar objetos
  por encima de capas donde el GBA los pondría detrás. Debe sustituirse o
  completarse antes de gameplay.
- E10 fue compilada y probada con GCC en PC; **no se pudo compilar con devkitARM
  ni ejecutar en melonDS dentro de este entorno**. Ambos siguen pendientes en
  Windows. No hay una ROM ni artefacto `.nds` en este ZIP.

## Cómo compilar y validar en devkitPro MSYS

Desde `zelda-tmc-3ds-main`:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Archivo esperado: `platform/dsi-engine/tmc-dsi-engine-e10.nds`.
Se busca `baserom_eu.gba` en `fat:/tmc-dsi/` o `sd:/tmc-dsi/`.
Los primeros segundos deberían mostrar los logos y el bosque como antes;
el diagnóstico en pantalla inferior reporta las llamadas de sprites. Es normal
que los gráficos OBJ no se vean todavía. START termina la prueba.

Pruebas en PC (si tienes compilador `cc`):

```sh
sh platform/dsi-engine/tests/run_title_e10.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e10_sanitized.sh /ruta/a/baserom_eu.gba
```

## Próximo bloqueo a resolver sin recreación

Enlazar el cargador de gráficos OBJ usado por el motor original, incluido el
ciclo de `CreateObject(TITLE_SCREEN_OBJECT)` y su carga de tiles. Una vez las
piezas apunten a tiles reales en `0x06010000`, podremos comprobar el sprite
original en la vista previa y en melonDS. Después deben venir el sistema de
fades, sonido y el bucle original de `AgbMain()`, con pruebas de cada subsistema.
