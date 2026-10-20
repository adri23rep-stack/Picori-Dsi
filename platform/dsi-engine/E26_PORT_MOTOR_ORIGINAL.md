# E26 — Portar el motor original, no recrearlo

La pantalla de la casa de Link en E25 demuestra lectura de ROM y visualización,
**no** que el motor jugable haya llegado a InitRoom(). El rectángulo oscuro
cerca de la puerta pertenece al escenario, no se considera defecto gráfico.

## Acuerdo técnico

- Mantener `src/` original como la única implementación de tareas, física,
  entidades, animaciones, mapa, cámara, combate y progresión.
- Escribir solo adaptadores de hardware (GBA -> DSi: memoria, gráficos,
  entradas, DMA, audio, guardado) y conversiones de datos estrictamente
  necesarias para las diferencias de plataforma.
- No inventar un bucle de movimiento de Link, motores de entidades o
  mapa paralelos para fingir jugabilidad.
- No incluir ROM comercial ni recursos originales en el repositorio.
- E25 permanece como referencia visual, E26 es rama de integración.
- Priorizar arranque del motor; audio y estabilidad completa después, pero
  mantener controles mínimos de seguridad de memoria para evitar corrupción.

## Primer cambio verificable

Bajo `TMC_DSI_E26_ORIGINAL_MESSAGE`, el DSi ahora enlaza el **archivo
original `src/message.c`** y ejecuta `MessageInitialize()` desde
`GameMain_InitRoom()`; el trace de título conserva el contador mediante
el adaptador de enlace `--wrap=MessageInitialize` que también llama
a la función original. Los estados aún no conectados continúan bloqueados.

Prueba host sin ROM: `sh platform/dsi-engine/tests/run_original_message_e26.sh`.
GitHub Actions compila un NDS a partir de estos cambios en la rama E26.

## Siguiente dependencia, sin atajos

Portar `InitRoom()` tal cual desde `src/gameUtils.c` y sus servicios
reales: estructuras de area, tablas de habitaciones, punteros a la ROM,
banderas y mapa. Después `InitUI()` y `InitializeEntities()` originales,
seguidos del estado `GAMEMAIN_CHANGEROOM`. No convertir el render de
diagnóstico E25 en un motor de juego alternativo.

Una compilación exitosa no garantiza el arranque; se requiere prueba en DSi
o melonDS antes de habilitar el siguiente estado.
