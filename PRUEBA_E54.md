# Picori DSi E54 — primera build para probar

Compilación ARM9: **SUCCESS**.
GitHub Actions: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39915256792
NDS: https://github.com/adri23rep-stack/Picori-Dsi/actions/runs/39915256792/artifacts/12484492117

## Para probar en melonDS

1. Descarga el artefacto `Picori-DSi-E54-Prueba` y descomprímelo.
2. Activa el modo DSi y la tarjeta SD virtual en melonDS.
3. Coloca una copia personal de la **ROM europea (BZMP)** del juego en la ubicación que ya utiliza la rama E41.
4. Abre `Picori_DSi_E54_Prueba.nds`. Presiona START para avanzar en la transición al cuarto.
5. Observa la habitación, el sprite de Link y, en la pantalla inferior, el diagnóstico **EU Link anim: ROM OK / BLOCKED**, si alcanzas la inicialización.
6. Si algo falla, guarda capturas de ambas pantallas e indica si se congeló, se cerró o salió `BLOCKED`.

La E54 es un **diagnóstico real**, NO una versión jugable: el original `PlayerInit` se compila pero no se fuerza su enlace/ejecución porque faltan 47 dependencias. Permanecen las validaciones de ROM, mapas, carga de habitación, sprite y el preflight de animación E53. No mover a Link con un bucle falso ni habilitar controles saltando scripts.

## Para continuar

- Rama de prueba: `e54-melonds-test-build`.
- Rama completa de desarrollo (enlace original todavía rojo): `e50-original-tile-player-bindings`.
- Rama previa estable: `e45-original-player-init-object`.
- Build de prueba: `make -C platform/dsi-engine TMC_DSI_TEST_BUILD=1`.
- Build de port completo: `make -C platform/dsi-engine` (todavía falla al enlazar).
- A futuro: portar `InitAnimationForceUpdate`, `UpdateAnimationSingleFrame`, scripts de despertar, OAM y servicios faltantes; solo después activar la actualización original del jugador.
