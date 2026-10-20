# E8 — Transicion original de logos al fondo del titulo (EU)

## Resultado comprobado en PC

- `TitleTask()` sin modificaciones ejecuta tres pasos iniciales y luego 122 llamadas adicionales hasta iniciar la escena siguiente (paso 125).
- El temporizador de 120 fotogramas se decrementa en el codigo **original** de `src/title.c`.
- Se cargan los grupos de graficos 2 y 3, y la paleta 4, directamente de `baserom_eu.gba` mediante cache SD; se conserva la logica original de seleccion.
- La escena original europea configura BG0/BG1 y el coeficiente de mezcla 9/9. El nuevo adaptador de video compone ambas capas con transparencia de indice cero y el efecto alpha de GBA.
- Test PC con ROM EU: BG0 = 5,872 pixeles, BG1 = 38,400, mezcla = 5,872. Firma FNV1a de la ventana 240x160: `4f749a4a`.
- La imagen de prueba muestra el bosque original con los rayos de luz; **no incluye todavia el logotipo/sprites, audio ni fades reales**.

## Comportamiento previsto en melonDS (aun sin probar)

- Arranca con los logos Nintendo/Capcom (E7).
- Tras ~120 fotogramas de `TitleTask()` original, carga los datos de la primera escena del titulo y dibuja bosque/rayos en pantalla superior.
- El programa se detiene intencionalmente en este limite del motor, sin continuar la logica de juego, para evitar que servicios aun simulados alteren el estado.
- Pantalla inferior: `Title stage: FOREST`, `BG blend: 5872 px`, `Stub calls: 8` (cifra de la prueba PC), `AgbMain: NOT RUNNING`.
- START termina el test.

## Limites honestos

1. `AgbMain()` completo NO esta ejecutandose. `TitleTask()` se invoca desde un lazo de prueba nativo DSi.
2. Hay 8 llamadas a servicios todavia instrumentados al entrar al titulo (entidades, audio, paletas, etc.). Esos servicios no implementan el juego.
3. La transicion es funcional en prueba de PC, pero NO se ha compilado aqui con devkitARM ni probado en melonDS.
4. La composicion actual soporta dos BG regulares con alpha; no incluye sprites/OAM, BG afines, ventanas, efectos por linea ni animaciones completas.
5. La ROM no se incluye en los ZIP. Se necesita la copia EU de 16 MiB ya usada en las etapas anteriores.

## Pruebas

```sh
sh platform/dsi-engine/tests/run_host_test.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e5.sh
sh platform/dsi-engine/tests/run_title_e6.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_renderer_e7.sh /ruta/baserom_eu.gba
sh platform/dsi-engine/tests/run_title_e8.sh /ruta/baserom_eu.gba
```

## Compilacion devkitPro (Windows)

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado esperado: `platform/dsi-engine/tmc-dsi-engine-e8.nds`.

## Siguiente bloqueo real

Portar el sistema de sprites/OAM para dibujar el logotipo y animaciones del titulo, reemplazar los stubs de fade/paletas y despues preparar el arranque completo de `AgbMain()` con sus dependencias de sonido, guardado, eventos y memoria. No reescribir gameplay.
