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

## E30.5 — Asignación y registro ORIGINALES de entidades

Validación del usuario: E30.4 ejecuta en melonDS con
`InitRoom PASS N:OK`, `Link LINKED L:OK C:OK`,
`Room ROM VERIFIED P:OK B:OK` y confirmó `err:0`.

El generador `generate_entity_allocator_e30.sh` selecciona, **sin
reescritura de cuerpos** de `src/entity.c`:
- `GetEmptyEntity()`: escaneo y reciclaje originales del pool GBA.
- `GetEmptyManager()`: pool original de 32 managers (`Temp`).
- `GetEmptyEntityByKind()`: despacho original a ambos pools.
- `gEntityListLUT`: mapa de clases a listas del juego original.
Y de `src/room.c`: `RegisterRoomEntity()` original, con la
estructura `GenericEntity` y prioridad/lista del juego.

Prueba de host E30.5: registro auténtico de Link existente y, después,
asignación de NPC y manager auténticos en pools separados; se comprueban
listados/cuentas/prioridades y que **Link no sea duplicado ni mutado**.
La simulación de host se limita a servicios de borrado no alcanzados en
este caso y al `MemCopy` estándar. No se ejecuta `LoadRoomEntity()`.

Los módulos quedan **compilados como objetos ARM9**, no enlazados al
ELF de la consola hasta tener los demás servicios originales. El CI
audita ambos símbolos y comprueba que no se introdujeron funciones de
spawn sin resolver al ejecutable. La habitación, Link y los indicadores
E30.3 siguen igual; el título es E30.5. La creación de NPC reales en
melonDS aún no está activada: siguiente trabajo será resolver
`DeepFindEntityByID`, `ResolveCollisionLayer` y scripts/objetos
originales sin stubs engañosos.

## Resultado E30.5

- CI definitivo **PASS**: prueba host, ensamblado ARM9 y auditoría
  de `GetEmptyEntity`, `GetEmptyManager`, `GetEmptyEntityByKind`,
  `RegisterRoomEntity` y `gEntityListLUT`.
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39575097634
- Archivo .nds E30.5:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39575097634/artifacts/12334425258
- Respaldo estable anterior: `e30-4-ci-pass`. Trabajo vigente:
  `e30-original-room-properties`.
- **Aún sin probar E30.5 en melonDS**. Esperado:
  `TMC / DSi ENGINE E30.5`, `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK`, `GameTask: ROOM PREFIX` y
  contador `err:0`. La escena permanece vacía porque no se ejecutan
  los nuevos allocators/registro ni el bucle original.

## E30.6 — DeepFindEntityByID ORIGINAL ejecutado en ARM9

La E30.5 fue confirmada en melonDS por el usuario con
`TMC / DSi ENGINE E30.5`, `InitRoom PASS N:OK`,
`Link LINKED L:OK C:OK`, `Room ROM VERIFIED P:OK B:OK` y
`err:0`.

E30.6 extrae de **`src/entity.c` sin reescritura** la rutina
`DeepFindEntityByID`, añadiéndola al módulo que ya alberga la
validación original `Port_IsValidEntityAddr` y el límite de recorrido
de listas introducidos por el port de PC/3DS.

La propia `DsiGameNative_BootOriginalPlayer()` llama ahora a la
función auténtica en ARM9 para comprobar:
- búsqueda positiva de Link por su `kind` e `id`;
- búsqueda negativa de un ID distinto de jugador;
- conservación de `gEntCount=1`, `gManagerCount=0` y lista de Link.

Si falla alguna, incrementa el contador real de errores y no
marca `Link original: LINKED`. El indicador **`F:OK`** en
`GameTask: ROOM PREFIX F:OK` solo aparece tras las dos búsquedas
auténticas y cero errores.

Prueba de host extendida: búsquedas positivas/negativas en jugador,
NPC y manager originales sin alterar contadores. `LoadRoomEntity`
todavía NO corre; allocators y scripts siguen fuera del enlace del
ELF gracias al recorte por secciones. No se ejecuta el bucle normal
ni se renderiza Link. **No confundir búsqueda real con spawn real.**

## E30.6 — Validación CI y prueba solicitada

GitHub Actions host y ARM9: **PASS** (ejecución de prueba host original,
símbolo `DeepFindEntityByID` enlazado en ELF y
`DsiGameNative_OriginalEntityLookupReady` presente).
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39577117676

Artefacto `.nds`:
https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39577117676/artifacts/12333729173

**Pendiente de melonDS:** con la ROM EU el título debe ser
`TMC / DSi ENGINE E30.6`. Verificar
`InitRoom original: PASS N:OK`, `Link original: LINKED L:OK C:OK`,
`Room ROM: VERIFIED P:OK B:OK`,
`GameTask: ROOM PREFIX F:OK`, `err:0`. Si `F:--`,
`BLOCKED` o error, no habilitar el siguiente escalón. Sigue sin
haber NPC visibles ni movimiento: la búsqueda no ejecuta spawns.

## E30.7 — Registro original ejecutado en ARM9 (prueba transaccional)

El usuario confirmó E30.6 en melonDS con `F:OK` y `err:0`.
E30.7 **enlaza al ELF ARM9 la función auténtica**
`src/room.c::RegisterRoomEntity()` y la ejecuta sobre el pool
`gEntities[]` del motor, utilizando los `AppendEntityToList()` y
`DeepFindEntityByID()` originales. La función diagnóstica de
plataforma crea un registro **sintético SOLO para pruebas** y lo
elimina de la lista al instante. **No es una nueva rutina de
spawn**, no crea un NPC permanente ni cambia a Link.

La prueba de E30.7 exige que antes exista Link auténtico
y que `RoomResInfo.properties[0/1]` estén conectados a
bancos originales; después:
- crea un único registro efímero de prueba con el registro original;
- comprueba su lista, prioridad y `DeepFindEntityByID` real;
- restaura la lista 7, limpia su slot y vuelve a dejar
  `gEntCount=1`, `gManagerCount=0`;
- verifica que Link no cambió y que el registro temporal desapareció.

No ejecuta `LoadRoomEntity()` ni `InitializeEntities()`, no corre
scripts de GBA ni mueve o dibuja personajes. Las funciones originales
de asignación/sprite/colisión siguen pendientes de enlazarse para
crear NPC reales.

**Diagnóstico E30.7:** `GameTask: ROOM PREFIX F:OK R:OK` o `R:--` (sin ocultar `ROOM SCREEN`).
`err:0`, `F:OK`, `N:OK`, `B:OK` y `P:OK` deben conservarse.
CI debe verificar ejecución host y símbolo nativo auténtico.

## E30.7 — CI final y aceptación melonDS

- Compilación final con pruebas host y ARM9: **PASS**.
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39578401136
- Artefacto `.nds` E30.7:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39578401136/artifacts/12335202171
- Respaldo E30.6: rama `e30-6-ci-pass`.
- **Pendiente**: validar en melonDS `TMC / DSi ENGINE E30.7`,
  `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK`,
  `GameTask: ROOM PREFIX F:OK R:OK` y `err:0`.
  La casa seguirá sin personajes dibujados, sin NPC permanentes ni
  actualización del motor. Un `R:--` indica que no pasó la prueba
  transaccional; no avanzar al spawn original hasta depurarlo.

## E30.8 — Validación de LoadRoomEntity original y auditoría de ROM

E30.7 fue comprobada por el usuario en melonDS:
`GameTask: ROOM PREFIX F:OK R:OK`, `err:0`.

E30.8 añade dos avances separados y verificables:

1. **Prueba host auténtica de `LoadRoomEntity()` y
   `LoadRoomEntityList()` desde `src/room.c`**, enlazadas con
   los `GetEmptyEntityByKind`, `RegisterRoomEntity`,
   `DeepFindEntityByID` y `AppendEntityToList` originales.
   Dos registros de prueba usan el modo especial `0x10`, que salta
   las dependencias de inicialización sin sustituir lógica; otro
   comprueba el filtro original de duplicados `0x50`. Los
   fallbacks de hardware o scripts NO se simulan: trampas de test
   host fallan si la rutina intenta ejecutarlos. Nada de esto
   se introduce en el ejecutable DSi.
2. **`DsiRoomSpawnAuditE30` en ARM9** inspecciona los
   `EntityData` auténticos de la ROM EU ya materializados, enumera
   cantidad total, rutas que necesitan scripts, managers, detección
   de duplicados y resolución de capas de colisión. Solo LEE datos.
   El indicador `ROOM SCREEN: DISPLAYED A:OK` confirma que
   se inspeccionaron ambas listas, sin crear NPC.

La función `sub_0804AF0C()` original también se extrae íntegra,
compila para ARM9 y se audita en el objeto, pero permanece **fuera
del enlace ejecutable**: todavía requiere `StartCutscene`,
`Port_ResolveEntityScript` y servicios de hardware/colisión.

`GameTask: ROOM PREFIX F:OK R:OK` debe seguir bloqueado.
`A:OK` **NO significa que el spawn real sea seguro o jugable**.
Para ejecutar una entidad real hacen falta resolver punteros/scripts
y `ResolveCollisionLayer` sin stubs de gameplay, además del
renderizado original. Nunca saltar este bloqueo.

## E30.8 — CI final y validación pendiente

- E30.7 pasó en melonDS con `F:OK R:OK` y `err:0`; se conservó
  como respaldo en rama `e30-7-ci-pass`.
- E30.8 pasó pruebas host de cadena ORIGINAL
  `LoadRoomEntityList -> LoadRoomEntity -> GetEmptyEntityByKind ->
  RegisterRoomEntity`, incluyendo rechazo de duplicados. Las
  operaciones de scripts y colisión NO están conectadas en ARM9.
- Compilación ARM9 y auditorías: **PASS**.
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39581328681
- Descargar NDS experimental:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39581328681/artifacts/12336159535
- **Prueba pendiente en melonDS:** `TMC / DSi ENGINE E30.8`,
  `ROOM SCREEN: DISPLAYED A:OK`, `InitRoom PASS N:OK`,
  `Link original: LINKED L:OK C:OK`, `Room ROM VERIFIED P:OK B:OK`,
  `GameTask: ROOM PREFIX F:OK R:OK` y `err:0`.
  `A:OK` únicamente certifica lectura/clasificación segura de la
  fuente original EU; NO crea NPC. El siguiente hito requiere
  cerrar scripts, `ResolveCollisionLayer`, asignación y OAM.

## E30.9 — Pool original de managers en ARM9 y coordenadas originales en host

Validación E30.8 por el usuario en melonDS: `ROOM SCREEN:
DISPLAYED A:OK`, `GameTask: ROOM PREFIX F:OK R:OK`,
`N:OK`, `P:OK B:OK`, `err:0`.

`GetEmptyManager()` desde **`src/entity.c`** original se enlaza
ahora en el ELF real de ARM9 (antes solo objeto) y se ejecuta dos
veces contra el pool original de 32 managers; se verifica que
ambas lecturas devuelven el mismo slot libre, sin reservarlo,
alterar contadores, crear manager, borrar entidades ni modificar
a Link. La prueba exige los bloques A y R de E30.7/E30.8.

Nueva prueba host ejecuta el **`sub_0804AF0C()` auténtico** extraído
literalmente de `src/room.c` para las ramas sin scripts:
- flags `0x00`: posiciones = ROM locales + `origin_x/y`;
- flags `0x20`: original marca `EM_FLAG_CAPTAIN` y añade origen;
- flags `0x10`: no modifica posición.

`0x40` script sigue fuera de pruebas y fuera del ARM9; cualquier
intento host de llamar scripts/deletion hace fallar la prueba.
`GetEmptyEntity`, `GetEmptyEntityByKind`, `LoadRoomEntity` y
`sub_0804AF0C` permanecen sin enlazar al juego completo, pues
faltan sus dependencias. Sin NPC persistentes ni movimiento.

**Marcador nuevo** de validación en melonDS:
`ROOM SCREEN:DISPLAYED A:OK M:OK`, indicando M:OK solo cuando
`GetEmptyManager` original retorna un slot válido sin tocar Link.
Se mantienen `GameTask: ROOM PREFIX F:OK R:OK`, `N:OK`, `P:OK B:OK`
y `err:0` esperados.

## E30.9 — Resultado CI y criterios melonDS

- **E30.8 ya validada** en melonDS por el usuario con
  `ROOM SCREEN: DISPLAYED A:OK`, `GameTask: ROOM PREFIX F:OK R:OK`
  y `err:0`.
- Estado anterior congelado en rama `e30-8-ci-pass`.
- CI **PASS**, pruebas host y compilación ARM9 de
  `GetEmptyManager` **auténtica enlazada al ELF**:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39582696185
- NDS experimental E30.9:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39582696185/artifacts/12338650136
- **Pendiente de ejecución real en melonDS**. Verificar:
  `TMC / DSi ENGINE E30.9`,
  `ROOM SCREEN:DISPLAYED A:OK M:OK`,
  `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK`,
  `GameTask: ROOM PREFIX F:OK R:OK` y `err:0`.
- `M:OK` sólo acredita búsqueda read-only de un slot original de
  manager. **No permite activar `LoadRoomEntity` ni las llamadas
  `StartCutscene`/hardware GBA**, que siguen pendientes de portar.

## E30.10 — Asignador original de jugadores auxiliares, compilado y ejecutado

E30.9 confirmado por el usuario en melonDS: `M:OK`, `A:OK`,
`F:OK R:OK`, `N:OK`, `P:OK B:OK`, `err:0`.

E30.10 extrae **sin alterar ningún cuerpo** la función
`src/entity.c::CreateAuxPlayerEntity()`, enlazándola al ELF ARM9
y ejecutándola dos veces después de los controles previos.
La función auténtica busca un `GenericEntity` auxiliar cuyo campo
`prev` esté vacío y, en builds PC_PORT, limpia ese slot.
El puente confirma **antes** de llamar que los siete slots ya
están completamente vacíos: no borra nada que el juego utilice.
No se registra un nuevo Link ni una entidad auxiliar permanente.

En host se prueba también el agotamiento original de los siete slots
y la recuperación del primero libre, restaurando todo después.
El marcador `U:OK` aparece en la misma línea que `size`,
`a:22` y `r:15`. Se preservan `F:OK R:OK A:OK M:OK`
y `err:0`.

**Importante:** E30.10 NO cierra la etapa de gameplay y
no activa `LoadRoomEntity`. Los servicios auténticos de borrado,
scripts, resolución de colisiones, sprites y bucle aún faltan.
No se crearon comportamientos Zelda alternativos.

## E30.10 — Prueba completa y aceptación en melonDS

- E30.9 validada por el usuario: `A:OK M:OK F:OK R:OK`, `err:0`.
- Respaldo de código E30.9: `e30-9-ci-pass`.
- Pruebas originales en host y devkitARM: **CI PASS**:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39647189869
- Archivo experimental `.nds` E30.10:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39647189869/artifacts/12368261418
- **Pendiente prueba en melonDS**: verificar `TMC / DSi ENGINE E30.10`,
  la línea `size:... a:22 r:15 U:OK`,
  `ROOM SCREEN:DISPLAYED A:OK M:OK`,
  `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK`,
  `GameTask: ROOM PREFIX F:OK R:OK` y `err:0`.
- `U:OK` solo acredita la ejecución real del asignador original de
  auxiliares sobre un pool comprobado previamente vacío. No se crea
  jugador auxiliar permanente; el GameTask UPDATE sigue bloqueado.

## E30.11 — Búsquedas completas ORIGINALES ejecutadas en ARM9

El usuario confirmó `U:OK` y `err:0` en la E30.10; respaldo
congelado en `e30-10-ci-pass`.

E30.11 añade al ejecutable ARM9 **cuatro funciones originales**
sin cambios a sus cuerpos (`src/entity.c`):
`FindEntityByID`, `FindEntity`, `FindNextDuplicateID` y
`EntityHasDuplicateID`. No se usan stubs falsos para estas
funciones. El diagnóstico comprueba las búsquedas en las listas
REALES de Link, incluido el rechazo de resultados equivocados,
y verifica que los contadores y enlaces no cambian.

En host también se comprueban duplicados en dos NPCs de prueba
usando las listas y `AppendEntityToList` originales. **Estos
NPCs son exclusivamente datos de pruebas en PC, nunca incluidos
en la lógica de la NDS.**

El nuevo estado `S:OK` aparecerá en la línea
`sz:240x160 a:22 r:15 U:OK S:OK`, conservando
`GameTask: ROOM PREFIX F:OK R:OK`, `ROOM SCREEN:DISPLAYED A:OK M:OK`
y `err:0` esperados. Ninguna rutina nueva ejecuta
`LoadRoomEntity`, `InitializeEntities` ni actualizaciones de NPC.

**Advertencia de alcance:** esto integra infraestructura auténtica,
pero **no desbloquea por sí solo la creación real de entidades**.
El siguiente corte imprescindible es cerrar `GetEmptyEntity`
junto a sus servicios auténticos de reciclaje/borrado, y
`StartCutscene`/`ResolveCollisionLayer` para las ramas
relevantes de `LoadRoomEntity`, sin recrearlos.

## E30.11 — CI y criterios finales de aceptación

- E30.10 validada en melonDS por el usuario con `U:OK`, `err:0` y todos los
  indicadores previos; respaldo en rama `e30-10-ci-pass`.
- GitHub Actions host y ARM9 **PASS**, incluyendo 4 símbolos auténticos
  `EntityHasDuplicateID`, `FindEntityByID`, `FindEntity`,
  `FindNextDuplicateID` **enlazados en ELF ARM9**:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39650505542
- NDS experimental:
  https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39650505542/artifacts/12370300574
- **Pendiente de melonDS:** esperar `TMC / DSi ENGINE E30.11`,
  `sz:240x160 a:22 r:15 U:OK S:OK`,
  `ROOM SCREEN:DISPLAYED A:OK M:OK`,
  `InitRoom original: PASS N:OK`,
  `Link original: LINKED L:OK C:OK`,
  `Room ROM: VERIFIED P:OK B:OK`,
  `GameTask: ROOM PREFIX F:OK R:OK` y `err:0`.
  `S:OK` es la búsqueda original sobre Link existente, no una prueba
  de NPCs originales ejecutándose en el bucle. En caso de `S:--`, no
  avanzar a creación real hasta identificar el bloqueo.
