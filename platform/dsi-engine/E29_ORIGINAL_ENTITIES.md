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
