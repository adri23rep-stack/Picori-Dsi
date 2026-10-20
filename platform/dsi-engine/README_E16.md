# Nintendo DSi — Engine E16 (motor original, todavía no jugable)

## Avances comprobados

1. `TitleTask()` y el botón START/A siguen llegando al `TASK_FILE_SELECT` original. **No se llama todavía a `FileSelectTask()`.**
2. La preparación de partidas continúa siendo **solo RAM**. No se abre, repara ni escribe archivo `.sav` alguno.
3. Se usan las funciones intactas `sub_080503A8(5)`, `LoadOptionsFromSave()` y `SetFileSelectState()` de `src/fileselect.c`.
4. **E16:** se ejecutan también las llamadas originales `sub_080503A8(6)` y `sub_080503A8(15)` que la primera iteración de `FileSelectTask()` solicitaría cuando cambia el idioma. Es una ejecución *aislada* de esos dos ayudantes, NO del bucle del menú. Con `RegionDefaultLanguage()` de EU, se cargaron **13,824 bytes** adicionales de ROM en memoria virtual en la prueba de PC.
5. Se corrigió la implementación de `DsiGbaBus_Copy` para las copias RAM→RAM que cruzan el límite de las dos paletas virtuales del GBA. Se preservan las reglas de `memmove` si las regiones se solapan, en ambas direcciones. Se validan todos los rangos **antes** de empezar para impedir una copia parcial cuando la dirección es inválida.
6. Se reinician los contadores de entrada de selección de partida al volver a entrar, sin conservar estados de una sesión anterior.

## Compilación DSi (pendiente de verificar en una instalación real devkitARM)

Desde la raíz `zelda-tmc-3ds-main` dentro de la consola de devkitPro MSYS:

```sh
make -C platform/dsi-engine clean
make -C platform/dsi-engine
```

Resultado **esperado, no comprobado en este entorno**: `platform/dsi-engine/tmc-dsi-engine-e16.nds`.

ROM europea propia en `fat:/tmc-dsi/baserom_eu.gba` (la imagen de SD/DLDI de melonDS debe estar activada). Al presionar START/A cuando el título lo permita, el diagnóstico inferior debe indicar `File-select: GFX READY` y bytes de grupos 5, 9, 6 y 15. **El menú no es visible ni interactivo todavía.**

## Pruebas de PC realizadas

```sh
make -C platform/dsi-engine test-all-e16 ROM=/ruta/baserom_eu.gba
make -C platform/dsi-engine test-e16-sanitized ROM=/ruta/baserom_eu.gba
```

Pruebas incluidas: E3–E15, nueva E16 con ROM EU, 3 ranuras temporales, inicialización repetida, actualización de grupos según región, prueba ROM→RAM atravesando 4 KiB, RAM→RAM solapada entre BG y OBJ palettes, rechazo de una copia fuente inválida *sin alterar el destino*. Los detectores AddressSanitizer y UndefinedBehaviorSanitizer no detectaron errores en los adaptadores examinados. Las pruebas de PC no garantizan funcionamiento ARM9/DSi.

## Lo que NO hemos conseguido

- **No ejecuta `AgbMain()`**, `FileSelectTask()`, escenas jugables, guardado ni el sistema completo de entidades.
- No hay menús nuevos hechos a mano: se cargan datos auténticos de la ROM, pero aún no se dibuja selección de partida.
- El emulador de RAM no reemplaza todas las dependencias del hardware GBA. En particular, las direcciones numéricas 0x02000000–0x02FFFFFF del GBA se superponen a direcciones físicas usadas por la RAM nativa de DSi: no es seguro tratar indiscriminadamente todos los punteros numéricos como si fueran punteros del GBA. Antes de activar `FileSelectTask()` completo hay que aislar esa diferencia y completar los servicios originales que necesita (mensajes, OAM, objetos, fade, guardados y audio).
- No se incluyen ni distribuyen ROMs ni assets extraídos.

**Siguiente meta:** enlazar funciones originales adicionales del inicio del menú, con una barrera explícita de llamadas a guardado y un mapa de dependencias, antes de intentar llamar al despachador `FileSelectTask()` completo.
