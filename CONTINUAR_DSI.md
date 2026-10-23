# Continuación del port nativo DSi

Rama de trabajo actual: `e44-original-player-arm9` (derivada de `e41-original-interior-collision`).
Base conservada: E40 `4285796d17214a11bbb0a35f7300afe90eda725b`.
Último código y pruebas de esta sesión:
`8a09f0a4d035d2414b3cd7d12d234a4deb1888b8`.
Detalles y procedencia: [E41_HANDOFF](platform/dsi-engine/E41_HANDOFF.md).

## Reglas del usuario

- Portar el código original; no recrear movimiento, scripts ni gameplay.
- Guardar cada avance coherente en GitHub para que otra IA pueda continuar.
- Explicaciones breves; no forzar CONTROL_ENABLED ni simular servicios pendientes.
- La ROM europea es del usuario: nunca subirla al repositorio.

## Lo que ya está implementado

- Mapas de colisión y actTiles completos de ambas capas, con rutinas originales.
- Parámetros y máscaras por píxel leídos de la ROM EU con límites comprobados.
- Sondeo original de los ocho puntos de la hitbox.
- LinearMoveDirectionOLD, FixedMul y seno originales; deslizamiento contra paredes.
- UpdatePlayerMovement y sub_08079E58 enlazados en ARM9 junto con sus dependencias
  originales de dirección y corrección de penetración.
- Corrección de seguridad PC_PORT: DIR_NONE provocaba acceso fuera de la tabla de
  pendientes. Se reproduce con ASan; ahora cada índice está limitado.

Esto todavía NO es jugable. El boot no llama a esas entradas de movimiento.
La habitación y Link estático siguen usando la ruta E40/E41 existente.

## Validación

- CI del enlace completo: [39884554411](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884554411), aprobado.
- CI de la corrección DIR_NONE: [39884783443](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884783443), aprobado.
- CI del último checkpoint: [39884902430](https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39884902430), aprobado en host y ARM9.
- ASan/UBSan: 262144 combinaciones de paredes cardinales, 40 máscaras y cuatro
  lados de corrección, velocidades con signo, direcciones y estado detenido.
- Con la ROM EU local: todos los píxeles de ambas capas en ocho estados y
  10824 pasos reales de UpdatePlayerMovement en 328 posiciones despejadas.
- No se ha probado este avance en melonDS ni en hardware.

Comandos de host desde la raíz:

```sh
E42_SANITIZE=1 sh platform/dsi-engine/tests/run_collision_probe_e42.sh
E31_SANITIZE=1 sh platform/dsi-engine/tests/run_room_spawn_e31.sh /ruta/ROM_EU.gba
```

## E44 — compilar src/player.c original para ARM9, sin fingir jugabilidad

Se añadió `platform/dsi-engine/tools/compile_player_e44.sh`, que
compila íntegramente el `src/player.c` original como objeto ARM9, con las
mismas definiciones de plataforma y optimización que el motor DSi.
La comprobación exige que existan `DoPlayerAction`, `PlayerInit` y
`PlayerSleep` reales, y deja una lista de referencias todavía sin resolver.
**NO enlaza ese objeto al .nds, no llama ninguna acción y no activa controles.**
Así se pueden corregir dependencias sin romper el arranque estable.

CI: rama `e44-original-player-arm9`, workflow `dsi-e41.yml`.
Los archivos `e44-player-original-symbols.txt`,
`e44-player-unresolved.txt` y `e44-player-link-gap.md` forman parte de
`E41-build-evidence`, si las pruebas pasan. La última comparación
distingue símbolo en ELF de comportamiento completo.

## Siguiente trabajo real

Ya hay una revisión automática para orientar el siguiente bloque:

```sh
python3 platform/dsi-engine/tools/audit_player_e43.py
```

Compila el player.c original y sigue sus referencias, incluidas las tablas
de acciones de PlayerSleep. El CI guarda el resultado en el archivo descargable
`E43-player-dependencies`. Opcionalmente acepta `--elf ruta/al/juego.elf
--nm arm-none-eabi-nm` para distinguir símbolos presentes y ausentes.
Presente no significa implementado: puede ser un servicio bloqueado o incompleto.
La revisión se detiene al salir de player.c; no declara completo todo el motor.

Continuar con las dependencias de PlayerInit y PlayerSleep de src/player.c.
No saltar su inicialización para mostrar movimiento con botones.
La compilación de src/player.c a objeto en host ya funciona; sus relocaciones
confirman dependencias en sub_0806FDA0, LoadSwapGFX, InitAnimationForceUpdate,
CheckQueuedAction y la cadena original de items/superficies/animación.

El siguiente bloque gráfico necesita revisar port/port_animation.c (traducción
existente del ASM), transporte nativo de SpritePtr/animaciones y slots de GFX.
gGFXSlots ya tiene almacenamiento en dsi_title_trace.c: no duplicarlo.
PlayerSleep requiere además contexto de script, ExecuteScript y animación;
ExecuteScript sigue bloqueado explícitamente en la ruta E33.
La vista estática actual de Link sólo soporta sus primeros 50 frames: no
presentarla como un renderer completo de despertar/caminar.

Conservar el objeto original 0x4f como Archway; BedCover es 0x5b.
No volver a introducir el sprite magenta que E40 puso en cuarentena.

## Precaución al guardar

El checkout local de esta sesión es una selección de archivos, no un clon
completo del historial remoto. Los commits se publicaron con árboles GitHub
basados en el árbol anterior y actualización del ref con SHA esperado.
No subir ese historial local encima del remoto. Los archivos generated/
son intermediarios; no se versionan.
