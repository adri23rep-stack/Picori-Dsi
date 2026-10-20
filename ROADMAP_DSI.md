# Ruta de trabajo — Zelda: The Minish Cap nativo para Nintendo DSi

> Estado de referencia: 2026-10-20. Hoja de ruta técnica, no calendario.
> **NO recrear el juego. Portar el motor original desde `src/`.**

## Objetivo

Hacer funcionar *The Legend of Zelda: The Minish Cap* de forma nativa en Nintendo DSi.
El código original del juego se encargará de movimiento, física, entidades,
mapas, lógica, progresión, cámara, eventos y menús. Los módulos específicos
de DSi son **adaptadores del hardware y de los datos**, no implementaciones
alternativas del juego.

**Primer hito jugable:** Link aparece y puede caminar dentro de su casa usando
el motor original, con animaciones y colisiones reales.

## Estado de partida (verificado)

| Versión | Estado |
| --- | --- |
| E25 | Compilación ARM9 exitosa, usuario confirmó que se ve la casa de Link (vista renderizada desde la ROM, no motor jugable). |
| E26, rama `e26-original-engine` | Compilación ARM9 y prueba host correctas para `MessageInitialize()` original de `src/message.c`. Quedan sin integrar `InitRoom()`, `InitUI()`, `InitializeEntities()` y actualización de juego por fotograma. E26 aún requiere prueba real en DSi/melonDS. |

E25 debe conservarse como **punto de restauración** hasta que E26+ supere sus pruebas visuales. La captura de la casa no equivale a que se esté ejecutando `InitRoom()`.

## Plan por hitos

### E27 — Capa de compatibilidad del motor de GBA con DSi

**Integrar, no simular:** memoria GBA/DSi, direccionamiento original,
tablas y punteros de ROM, variables globales, estructuras de 32 bits,
administración de memoria, DMA y funciones mínimas de acceso a hardware.
Usar el lector ROM y los servicios DSi ya presentes donde sean útiles.

**Criterio de salida:** pruebas host de lectura de ROM/memoria con límites,
compilación .nds en Actions y pantalla de E25 aún funcional; cada servicio
crítico tiene una implementación real o permanece bloqueado explícitamente.
No aceptar stubs silenciosos como si fuesen trabajo completado.

### E28 — `InitRoom()` original

Compilar y ejecutar `InitRoom()` de `src/gameUtils.c`, con sus dependencias
reales de áreas, metadatos, flags, mapas y recursos. Inicializar habitación
0x22/0x15 desde el estado original de nueva partida, no a base de datos
codificados para el renderizador de E25.

**Criterio de salida:** traza verificable de ejecución de `InitRoom()` original
en DSi/melonDS, área y habitación correctas, y ninguna dependencia esencial
marcada como omitida.

### E29 — Jugador y entidades del motor

Portar `InitUI()`, `InitializeEntities()` (en `src/game.c`) y las funciones
reales a las que llaman, incluido `InitializePlayer()` (en
`src/gameUtils.c`), lista de entidades, cámara y carga de habitación.

**Criterio de salida:** el motor original crea la entidad de Link, asigna
estado y coordenadas válidas, y realiza al menos una actualización real de
entidades; la renderización puede seguir siendo incompleta.

### E30 — Bucle normal del juego

Cambiar el diagnóstico de `GameTask()` por la ejecución recurrente del
motor, transitando por `GAMEMAIN_CHANGEROOM` y `GAMEMAIN_UPDATE`;
conectar entrada DSi -> `ReadKeyInput()` original y sincronización de cuadros.
Adaptar servicios originales de animación, entidades, colisión y cámara en
lugar de escribir una actualización paralela.

**Criterio de salida:** el juego avanza cientos de cuadros en la habitación
inicial con tareas originales, sin bloqueos artificiales de un solo tick,
sin escrituras fuera de memoria y con estados auditables.

### E31 — Salida gráfica del motor original

Enlazar fondos, mapas, sprites de entidades, OAM, paletas, desplazamiento y
DMA del motor con la pantalla superior de DSi. El renderizador de diagnóstico
E25 quedará solo como prueba de referencia y no como núcleo del juego.

**Criterio de salida:** Link, habitación y sus animaciones se muestran en
tiempo real según el estado del motor.

### E32 — PRIMER HITO JUGABLE: caminar dentro de la casa

Conectar la cruceta y entradas a los controles originales y validar el
movimiento, animaciones y colisiones reales de Link con paredes y muebles.

**Criterio de salida:** el jugador puede mover a Link en las cuatro
direcciones de la habitación inicial sin usar física ni un jugador ficticio.

### E33 — Cambios reales de habitaciones

Integrar puertas, transiciones, recarga de áreas, cámara y salida/entrada de
habitaciones usando el código original.

**Criterio de salida:** salir de casa, entrar a una habitación vecina y
volver sin reiniciar ni corromper el estado del juego.

### E34 — Interacción, eventos y menús

Integrar conversaciones, objetos, inventario, pausa, interacción con NPC,
scripts, escenas y progresión originales.

**Criterio de salida:** poder interactuar con un NPC/objeto, abrir el menú y
recuperar control del juego sin desincronizar el estado.

### E35 — Guardado

Integrar lectura/escritura de archivos en SD mediante adaptadores DSi y
conservar el formato lógico de guardado original cuando sea viable.

**Criterio de salida:** guardar, reiniciar y cargar conservando estado,
ubicación e inventario. Pruebas de integridad de archivo y apagado.

### E36 — Audio y estabilidad integral

Conectar música y efectos de sonido, resolver congelamientos, optimizar
rendimiento y memoria, revisar fallos de transición.

**Criterio de salida:** juego fluido y estable durante sesiones largas.

### E37+ — Juego completo y pulido

Cubrir todas las zonas, enemigos, jefes, cambios Minish, menús, escenas y
final. Optimizar la experiencia en hardware real.

**Criterio de salida:** recorrido completo comprobado, sin errores que
impidan progresar.

## Reglas innegociables

1. Mantener la lógica original en `src/`; no crear un segundo motor, física
   alternativa, IA sustituta ni movimiento de Link falso.
2. Los archivos `platform/dsi-engine/` deben servir para adaptación
   GBA -> DSi, instrumentación y pruebas, no para recrear jugabilidad.
3. Ejecutar el código original **por etapas**, haciendo explícito qué
   servicios siguen pendientes. No desbloquear a ciegas todo el motor.
4. Cada etapa requiere prueba host + GitHub Actions ARM9 + prueba visual
   en melonDS o DSi antes de considerarla terminada.
5. Un .nds compilado no equivale a un arranque exitoso, y una habitación
   dibujada no equivale a motor jugable.
6. No subir ROM `.gba`, BIOS, saves privados ni recursos comerciales al
   repositorio. Los usuarios aportarán su copia compatible (EU en las etapas
   actuales) fuera de Git.
7. No perseguir audio o guardado antes del hito E32, excepto si son una
   dependencia indispensable de inicialización; en tal caso hacer el mínimo
   puente real y continuar.
8. Conservar respaldo funcional y separar cambios de riesgo en ramas/PR.

## Ciclo fijo de trabajo para cada etapa

- **Inspeccionar:** código C original, dependencias y APIs GBA empleadas.
- **Adaptar:** integrar fuente original y proporcionar adaptador DSi
  únicamente donde el hardware difiere.
- **Compilar:** ejecutar CI host y devkitARM en GitHub.
- **Probar:** revisar logs y captura/vídeo real de melonDS o consola DSi.
- **Aceptar/reparar:** comprobar criterio de salida; si falla, arreglar
  errores sin fingir que el hito está completo. Conservar versión anterior.
- **Documentar:** anotar commits, problemas abiertos y siguiente dependencia.

## Próxima acción concreta

**E27.1:** auditar llamadas y globals que necesita `InitRoom()`, mapear
los punteros de GBA y los símbolos importados, priorizar los adaptadores
mínimos y preparar pruebas automatizadas. Luego E27.2: integrar la primera
dependencia original que desbloquee `InitRoom()`. No activar de golpe
el bucle completo.
