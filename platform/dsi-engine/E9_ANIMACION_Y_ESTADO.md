# Engine E9 — Animacion ORIGINAL de los rayos de la pantalla de titulo

## En que avanzamos realmente

**No es una recreacion del juego.** Los cuadros y cambios de color se obtienen
llamando a la funcion original `TitleTask()` de `src/title.c` una vez por VBlank.
La funcion original cambia las paletas mediante `LoadPaletteGroup(5..8)` y los
coeficientes `gScreen.controls.alphaBlend` en sus intervalos nativos. E9
simplemente detecta cambios en ese estado y vuelve a componer BG0/BG1 desde la
VRAM virtual, trasladando el resultado a la pantalla superior de Nintendo DSi.

Nuevos archivos: `source/dsi_title_live.[ch]` y
`tests/engine_e9_live_test.c`, `tests/run_title_e9.sh`,
`tests/run_title_e9_sanitized.sh`. `src/title.c`, `src/common.c`,
`src/flags.c` y `src/main.c` **no se modificaron**.

## Pruebas ejecutadas en este entorno

Con la ROM EUR proporcionada por el usuario:

- Regresion E3 (controles, flags, RAM, ROM paginada): PASS.
- Regresion E5 (tres transiciones originales): PASS.
- Regresion E6 (grupos graficos/paletas, LZ77): PASS.
- Regresion E7 (BG2, logos Nintendo/Capcom): PASS.
- Regresion E8 (BG0/BG1, logo -> bosque): PASS.
- E9, 170 ticks originales adicionales de `TitleTask()`: PASS.
- E9: 22 dibujos en total (primero + 21 cambios), 21 cambios de paleta,
  5 cambios de mezcla, 21 hashes visibles diferentes consecutivos,
  149 cuadros sin recomponer, sin errores del lector de ROM: PASS.
- E9 con AddressSanitizer + UndefinedBehaviorSanitizer en los adaptadores,
  lector y prueba (codigo original compilado sin instrumentacion): PASS.

La primera imagen del bosque sigue dando la firma E8 `4f749a4a`,
para garantizar que E9 no rompio la composicion conocida.

## Comportamiento previsto en DSi / melonDS

La misma secuencia de logos y bosque E8. Una vez en el bosque,
los rayos deberian cambiar periodicamente de color y transparencia segun
el **codigo original**. La pantalla inferior muestra ahora contadores
`draws`, `pal` y `alpha`. Se detiene intencionalmente despues de 300
llamadas adicionales a `TitleTask()` para no llegar al selector de partidas
sin sus dependencias. START sale del diagnostico.

## Como probar

En devkitPro MSYS, desde la raiz del proyecto:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado esperado: `platform/dsi-engine/tmc-dsi-engine-e9.nds`.
No pudimos compilar con devkitARM en este entorno: aun es necesario
compilarlo en Windows y verificar que inicie en melonDS DSi.

ROM EU de 16 MiB (NO incluida): `fat:/tmc-dsi/baserom_eu.gba`
(o `sd:/tmc-dsi/baserom_eu.gba`).

Pruebas en PC, con C/GCC/Clang y ROM europea propia:

```sh
sh platform/dsi-engine/tests/run_title_e9.sh /ruta/a/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e9_sanitized.sh /ruta/a/baserom_eu.gba
```

## Lo que NO esta resuelto

- `AgbMain()` no se ejecuta todavia. `TitleTask()` es invocado por un
  bucle de diagnostico de la DSi.
- Los logos estan formados por el fondo original; faltan los sprites OAM,
  el emblema/espada, animaciones complejas, sonido y fades de GBA.
- Servicios de motor todavia instrumentados: `DrawDirect`, `CreateObject`,
  `SoundReq`, `UpdateEntities` y otros. Las cifras de `unsupported_service_calls`
  NO deben interpretarse como funciones implementadas.
- La representacion de ROM es paginada, pero las referencias directas
  a punteros de ROM del port de PC necesitan adaptacion sistematica antes
  de entrar en gameplay; DSi solo tiene 16 MiB de RAM total.
- El rendimiento y timing real del dibujado requieren una prueba en melonDS
  y finalmente en consola DSi. No se promete 60 FPS ni jugabilidad.

## Siguiente fase tecnica

1. Portar el generador original de sprites (partes de `src/affine.c` y
   rutinas `ram_DrawDirect` / `ram_sub_080ADA04`) hacia OAM/obj virtuales.
2. Incorporar la composicion de OBJ por prioridad, transparencia y paletas.
3. Integrar el sistema de fades/mezcla de la pantalla de titulo sin usar
   las funciones simuladas E9.
4. Separar el arranque de `AgbMain` por subsistemas y sustituir cada
   llamada instrumentada por un backend real, manteniendo intactas las
   reglas y las tareas del motor.
