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

## E30.1 — Fuente de propiedades de habitación ORIGINAL (lectura segura)

- `dsi_room_properties_e30.c` usa `DsiRoomMetadata.properties_address`
  resuelto por el motor de tablas original (área 0x22, habitación 0x15).
  Copia exactamente las ocho entradas de 32 bits a RAM ARM9.
- Valida origen ROM EU, legibilidad de cada dirección no nula y que las
  propiedades 0/1 coincidan con los dos bancos de entidades que ya se
  verificaron en E28. Clasifica direcciones Thumb (bit 0) como *datos de
  origen*, NUNCA como callbacks ejecutables en DSi.
- Los valores de GBA no se guardan en `RoomResInfo.properties`: todavía
  falta materializar propiedades nativas y las tablas de callbacks/guiones
  originales. El gate `DSI_E30_BLOCK_PROPERTIES` debe seguir bloqueado.
- Los cambios son atómicos: una lectura defectuosa conserva el snapshot
  anterior. La prueba host usa bytes sintéticos y prueba corrupción,
  alineación, direcciones inválidas y fallas de lectura.

**Este paso no activa `InitializeEntities()` ni `GameTask UPDATE`,
ni sustituye scripts. La pantalla seguirá con habitación y Link invisible
hasta que se porte el renderizado real de las entidades.**

## Rama de trabajo y bloqueo seguro

La rama activa de E30 es `e30-original-room-properties`. 
La versión E29.8 probada anteriormente queda conservada como
`e29-8-ci-pass` en el commit `cf29b734` (Actions PASS).

Desde E30.1 el guard `DsiE30Gate_Blockers()` exige **dos etapas**
para aceptar propiedades: snapshot ROM original válido y misma habitación;
luego el despacho nativo realmente verificado más
`RoomResInfo.properties` materializado. El primer requisito ya tiene
código y pruebas; el segundo sigue **bloqueado a propósito**.
El test host verifica ambos casos y nunca habilita `GameTask UPDATE`.

## E30.2 — Bancos originales EntityData materializados en ARM9

La ROM EU ya se leyó en E30.1. El paso siguiente usa **solo** las
propiedades originales 0 y 1 (`EntityData[]`). Se comprueba la
coincidencia con `DsiEntityLists`, se copian íntegros los registros
y el terminador `0xFF` a estructuras C nativas **alineadas**, con
`_Static_assert(sizeof(EntityData)==16)` para proteger el layout GBA.

`DsiRoomPropertyDataE30_EntityBank(0/1)` expone const datos originales
preparados para el futuro `LoadRoomEntityList()` del `src/room.c`, pero
**no lo llama**. Las direcciones spritePtr originales **NO** son
punteros nativos y no se desreferencian. Los campos 2–7 todavía pueden
ser datos/scripts/callbacks GBA sin equivalentes nativos y siguen bloqueados.

`DsiE30Gate_Blockers()` agrega `DSI_E30_BLOCK_ENTITY_BANKS`. Debe
seguir bloqueando `GameTask UPDATE` hasta enlazar
`InitializeEntities()`, callbacks y render original. Al cargar en melonDS
se espera `Room ROM: VERIFIED P:OK B:OK` y el resto de E30.1 sin cambios.

- Validación host: coincidencia byte-a-byte, alineación de `EntityData`,
  terminadores, cambio de sala, datos corruptos, snapshot anterior estable.
- Validación ARM9: objeto nativo y símbolos presentes en ELF.
- Prueba host E30.2 y compilación ARM9: **PASS**.
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39566359126
- Artefacto NDS E30.2:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39566359126/artifacts/12330361089
- **Pendiente:** confirmar en melonDS `B:OK`, Link `LINKED L:OK C:OK`,
  InitRoom PASS y `err:0`. Sin esto no declarar ejecución validada.

## E30.3 — Propiedades 0/1 en la estructura original RoomResInfo (preparadas)

La evidencia del usuario en melonDS confirmó `InitRoom original: PASS`,
`Link original: LINKED L:OK C:OK` y `Room ROM: VERIFIED P:OK B:OK`
para E30.2. No se recibió el valor de `err`, así que no se presupone.

Con E30.3, tras el `InitRoom()` y el `InitializePlayer()` originales, la
capa de plataforma **solo** escribe `RoomResInfo.properties` apuntando
a un arreglo nativo de ocho slots. Los slots 0/1 son punteros estables a
`EntityData[]` originales, verificados en E30.2; slots 2–7 quedan
expresamente NULL. Se conserva intacto el código de `src/gameUtils.c`
y las entidades **no** se instancian ni actualizan.

El nuevo `DsiRoomPropertyNativeE30_Ready()` confirma que
`gArea.pCurrentRoomInfo` es exactamente la sala original correcta,
sus dimensiones, el puntero instalado, y que slots 2–7 permanecen
vacíos (sin callbacks Thumb de GBA). Rechaza estados sucios, corrupción,
otras habitaciones y doble instalación. El reset solo retira
`properties` si pertenece a este adaptador.

`DsiE30Gate_Blockers()` añade `DSI_E30_BLOCK_NATIVE_ROOM_PROPS`.
**E30 sigue bloqueado:** faltan portear scripts/callbacks 2–7,
`InitializeEntities()` y el dibujado de entidades de OAM; no ejecutar
`GameTask UPDATE`. El nuevo diagnóstico de E30.3 será
`InitRoom original: PASS N:OK` junto a `P:OK B:OK`.

**Pruebas host y ARM9 E30.3: PASS** — https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39567279884

**NDS E30.3:** https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39567279884/artifacts/12329673108

**Pendiente:** prueba en melonDS con ROM EU, `InitRoom original: PASS N:OK`, `Link original: LINKED L:OK C:OK`, `Room ROM: VERIFIED P:OK B:OK`, `err:0`. No hay movimiento ni sprites del jugador.

## E30.4 — ORIGINAL LoadRoomEntityList + LoadRoomEntity compilados en ARM9

La fuente base permanece **`src/room.c` intacta**. El generador
`generate_room_entity_core_e30.sh` extrae literalmente los dos cuerpos
originales, compilándolos para devkitARM en secciones independientes:

- `LoadRoomEntityList`: recorre cada `EntityData` hasta el terminador
  original `kind=0xff` y llama al original `LoadRoomEntity`.
- `LoadRoomEntity`: permanece **solo compilada en objeto**, sin invocar
  ni forzar su enlace al ejecutable hasta tener `GetEmptyEntityByKind`,
  `DeepFindEntityByID`, `RegisterRoomEntity`, resolución de scripts,
  colisiones y otros servicios originales.

La prueba host sí ejecuta `LoadRoomEntityList` desde la **estructura real**
`RoomResInfo.properties[0/1]` preparada por E30.3. Su único límite
simulado es la función de *spawn* `LoadRoomEntity`: un observador para
comprobar el orden de las llamadas, nunca incluido en el .nds.

En ARM9 se compilan ambos cuerpos y CI audita sus símbolos y las
dependencias pendientes; **no se ejecuta `LoadRoomEntityList` ni se
generan NPC**. El diagnóstico visible E30.3 no cambiará, salvo el
título E30.4. E30 sigue sin `GameTask UPDATE` y sin dibujar Link.

**Prohibido:** crear un `LoadRoomEntity` falso para facilitar el enlace.
La próxima etapa deberá cerrar las dependencias con funciones auténticas
de `src/entity.c`, `src/room.c` y el port ya existente.

## Evidencia E30.4 CI

GitHub Actions **host y ARM9 PASS**:
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39568450901

Se auditan los símbolos de los dos cuerpos **originales**, y CI confirma
que todavía no aparecen en el ejecutable final: permanecen en sus
objetos ARM9 hasta resolver las dependencias y no se fuerza un
`LoadRoomEntity` ficticio. La lista necesita adaptar la resolución
`Port_ResolveRegionData` para memoria DSi; `LoadRoomEntity` necesita
`GetEmptyEntityByKind` y `RegisterRoomEntity` reales, entre otros.

La pantalla muestra ahora **E30.4** en el título para diferenciar la
compilación, pero los resultados de habitación y de Link deben
mantenerse idénticos a E30.3. No hay NPC nuevos ni movimiento.

## E30.4 — Compilación final

- GitHub Actions final **PASS** (tests host, devkitARM y auditoría de
  dependencias sin falsos loaders):
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39568748309
- NDS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39568748309/artifacts/12330917807
- El título en la consola inferior ahora es `TMC / DSi ENGINE E30.4`.
  Las líneas `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK` y
  `GameTask: ROOM PREFIX` deben continuar sin cambio.
- **Pendiente melonDS**: comprobar diagnóstico y `err:0` en el .nds.
  La etapa no crea NPC ni dibuja a Link; eso sigue bloqueado.
