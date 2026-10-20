# E29 — Entidades y Link originales (en desarrollo)

## Regla del proyecto
**Portar el motor original de The Minish Cap; NO crear un jugador, motor de
colisiones ni lógica alternativa.** El render de la casa de E25/E28 sigue
siendo una vista de diagnóstico y no demuestra ejecución del juego completo.

## E29.1 — InitializePlayer original: test host aprobado
`run_original_player_e29.sh` extrae directamente el cuerpo intacto
`InitializePlayer()` de `src/gameUtils.c` y lo compila con los tipos
`Entity`, `PlayerState`, `PlayerEntity`, `RoomTransition`,
`SaveFile` auténticos.

Verifica salud, coordenadas de guardado, acción de aparición, bandera
`PL_NO_CAP`, cámara, tipo `PLAYER`, flags de colisión/persistencia
y la petición de alta en la lista de entidades. Los servicios de
cámara/sprite/listas se sustituyen **solo en pruebas host**. No se ha
agregado ningún personaje falso al `.nds`.

## E29.2 — Código original de listas y prioridades: test host aprobado
`run_original_entity_lists_e29.sh` extrae del `src/entity.c` real:

- `AppendEntityToList()`
- `InitDefaultPriority()`
- `SetEntityPriority()`
- `gPrioritiesDefault` y `gPrioritiesKinstoneMenu`

Comprueba punteros `prev/next` nativos, centinela real de lista,
conteo de entidades, vinculación de múltiples entidades y prioridad
original de jugador y NPC.

## E29.3 — Preflight nativo DSi
`dsi_player_preflight.c` comprueba el estado auténtico de nueva partida
antes de permitir `InitializePlayer()`:

- Área/sala actuales coinciden con `RoomControls`, `RoomTransition`,
  `DsiRoomResources` y `DsiEntityLists`.
- Tipo de aparición original válido, capa original plausible.
- Salud positiva y dentro del máximo, sin modificar `gSave`.
- Ambas listas de registros de entidades EU tienen longitudes y
  terminadores `0xFF` coherentes, obtenidos de la ROM del usuario.

El diagnóstico inferior mostrará `Link preflight: READY` o
`BLOCKED`. **READY no implica que Link haya aparecido.**
No se invoca `InitializePlayer()` en ARM9 todavía: faltan sus
servicios auténticos de listas, prioridad, sprites y hitbox.

## Dependencias para E29.4 en adelante
1. Vincular `src/entity.c` original y su asignador/listas de
   entidades en ARM9, sin crear entidades sintéticas y sin romper los
   hooks antiguos del título E25.
2. Conectar los servicios originales `ResetPossibleInteraction`,
   `UpdateSpriteForCollisionLayer`, `AppendEntityToList`,
   `RegisterPlayerHitbox`, memoria de `gActiveItems`, flags y
   prioridades reales.
3. Resolver `RoomResInfo.properties`, scripts y referencias a
   salidas/tiles que hoy siguen `NULL` o como direcciones GBA
   (no deben desreferenciarse directamente en ARM9).
4. Llamar a `InitializePlayer()` auténtica dentro de
   `InitializeEntities()` original, *solo* cuando los prerrequisitos
   estén conectados y verificados.
5. Dibujar sprite y animaciones a través del motor original; el
   movimiento y las colisiones se habilitarán cuando funcione el
   bucle normal del juego en las siguientes etapas.

## Criterio de prueba actual
- Tests host E25–E29: PASS.
- ARM9/Actions: verificar antes de dar por compilado.
- melonDS: E29 experimental debe seguir mostrando la casa y
  `InitRoom original: PASS` y enseñar `Link preflight: READY`.
- E28 queda intacto como último hito aceptado en melonDS.

Este trabajo no incluye la ROM comercial ni datos protegidos del juego.

## E29.3 — PRUEBA REAL EN MELONDS (confirmada por el usuario)

Captura aportada del `TMC / DSi ENGINE E29` utilizando su ROM EU:

- Casa de Link visible y con los gráficos correctos.
- `Area tables: PASS`, `Room cache: READY H:OK`.
- `InitRoom original: PASS`, `Room tiles: OK 86132 B`.
- `Link preflight: READY`.
- `GameTask: ROOM PREFIX`, `orig:1/1/1 init:1 d:10 e:0`.
- `Room pixels:38400 E:READY`.
- `entity:0 upd:0 err:0` — todavía **no hay entidades del juego creadas**.

Esto completa la **validación funcional de E29.3**, no la de E29
completa. La confirmación no incluye Link dibujado ni movimiento.
La versión E28 sigue preservada como respaldo probado.

## E29.4a — Primeras funciones originales de src/entity.c compiladas para ARM9

**Estado:** GitHub Actions PASS en
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39477764142
(sin prueba melonDS de este nuevo .nds).

Primero se intentó enlazar `src/entity.c` completo. El compilador C lo
aceptó, pero el enlazador detectó referencias reales todavía pendientes:
`gEntityLists`, `gEntities`, `gAuxPlayerEntities`, servicios gráficos,
hitbox, scripts, `ram_UpdateEntities` y registro de depuración PC. No se
solucionó añadiendo stubs engañosos ni un asignador falso.

Como avance seguro, el build genera `dsi_entity_core_e29.c`
**directamente de los cuerpos literales de `src/entity.c`** mediante
`tools/generate_entity_core_e29.sh`. No es una recreación de entidades.
Las funciones auténticas compiladas y comprobadas con `arm-none-eabi-nm`:

- `SetInitializationPriority()`
- `SetEntityPriority()`
- `InitDefaultPriority()`
- `AppendEntityToList()`
- Las tablas originales `gPrioritiesDefault` y `gPrioritiesKinstoneMenu`.

`SetInitializationPriority()` ya se **enlaza** desde esa fuente original
y reemplaza el adaptador temporal de E28 que tenía una copia de su
asignación. Las demás funciones están **compiladas en el objeto ARM9**;
aún no hemos demostrado la ejecución de `AppendEntityToList()` en el
juego real ni creado a Link. El enlazador puede descartar funciones que
todavía no son alcanzables durante esta etapa.

## Próximos pasos para E29.4b

1. Conectar almacenamiento de `gEntityLists`, `gEntities` y
   `gAuxPlayerEntities` del motor, inicializando centinelas originales
   y validando límites antes de darles uso.
2. Sustituir los hooks del título solo cuando no comprometan el arranque.
3. Incorporar las dependencias originales de `InitializePlayer()`
   (sprite/layer, hitbox, interacción y registro de entidades).
4. Resolver `properties`/scripts/ROM GBA antes de habilitar
   `InitializeEntities()`; nunca ejecutar direcciones ROM como código.
5. Probar en melonDS que E29.4a mantenga `InitRoom original: PASS`,
   `Link preflight: READY` y la casa visible.

**E29 completa continúa pendiente.** Este .nds todavía es un diagnóstico
con la casa renderizada; no es un prototipo jugable.

## E29.4b — Variables originales de entidades en memoria de DSi

**Compilación y pruebas CI aprobadas:**
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39485524004

`dsi_entity_state_e29.c` ahora aporta RAM real de ARM9 para las
estructuras exactas del código original:

- `LinkedList gEntityLists[9]`
- `GenericEntity gEntities[MAX_ENTITIES]`
- `GenericEntity gAuxPlayerEntities[MAX_AUX_PLAYER_ENTITIES]`
- `gEntCount` y `gManagerCount`

`DsiEntityState_Reset()` deja las nueve listas vacías con los
centinelas **originales**, referidos a sí mismos, y limpia el conjunto
de entidades con tipos originales. No se crean NPC ni Link y tampoco
se ejecuta un bucle de juego ficticio.

El test `run_entity_state_e29.sh` compila los cuerpos literales
originales de `src/entity.c` y demuestra que
`AppendEntityToList()`, prioridades y contadores manipulan esas
variables, restaurándose luego a su estado vacío.

El arranque DSi utiliza `DsiEntityState_Reset()` y comprueba
`DsiEntityState_EmptyValid()`. El diagnóstico añade `L:OK`
junto a `Link preflight: READY`. `L:OK` **solo** demuestra
preparación correcta de memoria/listas, no que haya entidades vivas.

**Estado real:** tests host y enlace ARM9 correctos. Aún falta
confirmar en melonDS que se mantengan habitación, inicialización
original, `L:OK` y errores cero. Se conserva E29.4a validada como
referencia. E29 completa NO está terminada.

**Próximas dependencias:** enlazar la asignación auténtica, hitbox,
colisión, gestión de sprites, scripts y referencias ROM necesarias
antes de ejecutar `InitializePlayer()`/ `InitializeEntities()`.
