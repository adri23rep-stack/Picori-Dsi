# E30 — Preparación del bucle original (NO iniciado)

## Punto de entrada único

`src/game.c::GameTask()` y los estados `GAMEMAIN_CHANGEROOM` y
`GAMEMAIN_UPDATE`. No introducir bucles alternativos en
`platform/dsi-engine`.

## Bloqueos medibles previos

1. E29: `InitializePlayer()` original debe ejecutarse en ARM9, con
   `UpdateSpriteForCollisionLayer()` resuelto y sin sustituto.
2. El jugador auténtico debe enlazarse en `gEntityLists[1]` exactamente
   una vez y estar apuntado por `gRoomControls.camera_target`.
3. `RegisterPlayerHitbox()` auténtico debe registrar al jugador real,
   sin punteros a la memoria temporal de los tests.
4. `InitializeEntities()` debe resolver sus propiedades y scripts
   ROM EU antes de `CallRoomProp6`, `LoadRoom`, etc.
5. `InitUI`, mapa, OAM, cámaras, tareas y pantalla: adaptar a DSi
   con accesos seguros a ROM, no reinterpretar punteros GBA como C.
6. Verificar en melonDS: habitación, `InitRoom original: PASS`,
   lista/cámara/hitbox, sin errores, sin cuelgues.

## Implementación E30 cuando E29 pase

- Activar la actualización REAL por fotograma con `GameTask()` de
  `src/game.c`, bajo guardas y límites de seguridad.
- Traducir teclas DSi a entrada original sin repetir lógica de movimiento.
- Probar primero 1, 10, 100, 300 fotogramas con trazas de estado,
  posición, entidades, fallos y memoria.
- En caso de fallo, parar antes del siguiente tick sin corromper
  guardados/ROM. No ejecutar guardado hasta E35.
- No declararlo jugable sin E31 (gráficos reales) y E32
  (control/colisiones del motor).

## Evidencia actual

E29.6 Actions PASS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39528402176

E29.7 introduce prueba encadenada en host; requiere resultado Actions.
**Nunca confundir host PASS ni .nds creado con Link visible.**

## E30 gate (implementado, pero bloqueado a propósito)

`dsi_e30_gate.c` solo revisa memoria **real** del motor: estado de
`GameTask`, sala, entidad de Link, lista, cámara, hitbox, interacción,
propiedades de habitación, finalización genuina de `InitializeEntities()`
y sprite/layer. No existe ninguna simulación de `GameTask` alternativa.

La fuente `src/game.c` notifica al gate **únicamente al terminar la función
original `InitializeEntities`**; actualmente no puede ocurrir debido al
arranque seguro. Mientras no se porte `UpdateSpriteForCollisionLayer`,
el bloqueo `DSI_E30_BLOCK_SPRITE_LAYER` es obligatorio.

La prueba `run_e30_gate.sh` verifica que incluso con variables ideales
en el test **E30 siga rechazado**. La guarda se reinicia en el arranque
y se compila en ARM9 sin habilitar ninguna actualización de juego.

**No confundir gate creado con E29 completado o E30 listo para ejecutar.**

## E29.8 / E30

En E29.8 se intenta ejecutar **InitializePlayer real** una sola vez
tras InitRoom, con hitbox y lista; no se ejecuta el resto de
InitializeEntities mientras properties/scripts y cámara estén pendientes.
El gate de E30 sigue rechazando el bucle y el movimiento del jugador.

La función sprite/layer de la fuente 3DS ya está disponible, pero la
habilitación completa del render OAM del juego corresponde a E31.

## Resultado verificable

E29.8 ARM9 compilada: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39550565870

El `DsiE30Gate_Blockers()` de la build E29.8 sigue bloqueando E30 si
faltan propiedades reales de sala, ejecución completa de
`InitializeEntities()` y la validación del sprite/layer nativo en melonDS.
Hasta resolverlo, `GameTask` permanece intencionalmente en modo seguro,
sin `GAMEMAIN_UPDATE`.
