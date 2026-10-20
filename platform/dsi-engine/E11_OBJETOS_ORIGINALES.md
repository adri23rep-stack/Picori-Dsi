# Engine E11 — título: ciclo real de `TitleScreenObject`

## Qué añade respecto de E10

- **Enlaza y ejecuta sin modificar** `src/object/titleScreenObject.c` (funciones
  `TitleScreenObject`/`TitleScreenObject_Type0/1/4`, con sus coordenadas,
  fotogramas, estado y temporizadores auténticos).
- Sustituye la anterior función ficticia `CreateObject()` por un **puente
  temporal de asignación acotado**, únicamente para `TITLE_SCREEN_OBJECT`.
  Los objetos que no son del título se rechazan explícitamente. Un máximo de
  8 entidades evita crecimiento sin control y no suplanta al gestor completo
  `src/objectUtils.c`.
- `EraseAllEntities()` vacía este almacén; `UpdateEntities()` ejecuta el
  comportamiento original, visitando los hijos creados a partir del siguiente
  cuadro. `DrawEntities()` sigue sin pintar nada: **faltan el constructor
  real de sprites, las texturas OBJ y el pipeline de entidades**.
- Usa el mismo algoritmo de `Random()` del GBA (ROR(state*3,13), retorno
  state>>1), a través del `port/port_rng.h` ya incluido en este proyecto; usa
  la semilla original 0x1234567. Se verificó una secuencia de ocho valores.
- Los errores del almacén de entidades detienen el avance de título en la
  demo DSi para evitar continuar sobre memoria dañada. Pantalla inferior:
  `entities:4 upd:... err:0`, una vez creada la estructura.
- No se altera ni una línea de `src/title.c`, `src/main.c`, `src/common.c`,
  `src/flags.c` ni `src/object/titleScreenObject.c`.

## Validación realizada en host con ROM EUR (del usuario)

`sh platform/dsi-engine/tests/run_title_e11.sh /ruta/ROM_EUR.gba`

Después de las 3 transiciones iniciales, 120 cuadros de logos, 170 cuadros
con luces y 560 cuadros extra:

- `TitleScreenObject` creó 4 entidades en total (principal tipo 0, hijos
  tipos 1, 3 y 4) sobre la ROM EUR con el valor inicial de EWRAM[7]=0.
- Actualizaciones originales: **2693** (674 del objeto principal, 2019 de
  los tres secundarios), sin desbordamientos ni errores de asignación.
- Se verificó también la otra rama original: EWRAM[7]=1 genera los tipos
  0, 1, 2 y 4. La actualización no procesa recursivamente los hijos creados
  en la misma pasada.
- Se forzó un intento de crear una novena entidad y se verificó el rechazo
  sin desbordar el almacén. Un objeto no soportado también se rechaza.
- La secuencia del RNG pasó los 8 vectores originales.
- E10 conserva 427 solicitudes directas de sprites, 1564 piezas procesadas,
  0 errores de lectura de registros de ROM, pero **0 píxeles de sprites
  auténticos** sin el cargador original de texturas.
- `run_title_e11_sanitized.sh` comprueba nuestros puentes con ASan/UBSan.
- Suite E3–E11 de PC: pasa tras integrar esta etapa.

## Limitaciones críticas

E11 **no es jugable**. Este puente NO es el gestor completo de entidades y no
admite enemigos, jugador, físicas, guardar/cargar partidas ni otras familias de
objetos. Tampoco hay fade auténtico ni audio. No se llama a `AgbMain()`.
El render del bosque sigue viniendo del mismo `TitleTask()` y la ROM EUR;
los objetos de la pantalla de título todavía no tienen texturas OBJ visibles.
La lectura de ROM es por caché SD, no emulación GBA por CPU.

Esta etapa fue compilada y probada con GCC de PC; **no se ha compilado ni
probado en hardware DSi/melonDS** porque no hay devkitARM instalado en este
entorno. No se incluye ROM ni binario `.nds`.

## Compilación para DSi en Windows con devkitPro

En la raíz del ZIP (`zelda-tmc-3ds-main`):

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Salida esperada: `platform/dsi-engine/tmc-dsi-engine-e11.nds`.
Coloca tu copia EUR `.gba` en la SD de melonDS como
`fat:/tmc-dsi/baserom_eu.gba` (también se intenta `sd:/...`).

## Prioridad siguiente

Portear/integrar el flujo de `ObjectInit`/`LoadFixedGFX` y `LoadObjPalette`
del código original para que los cuatro objetos del título realmente tengan
texturas en VRAM. Después sustituir el almacén temporal por el gestor real
`CreateObject` + listas y conectar el bucle general con `AgbMain()`.
