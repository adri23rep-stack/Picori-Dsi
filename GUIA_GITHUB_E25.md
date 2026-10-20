# E25: publicar en GitHub y compilar `.nds`

**Este paquete no contiene una ROM de Zelda ni un `.nds` ya compilado.**

1. En Windows, instala **Git** (https://git-scm.com/downloads/win) y **GitHub CLI** (https://cli.github.com/) si aún no los tienes.
2. Extrae *todo* el ZIP en una carpeta común. **No ejecutes el archivo `.bat` desde dentro de la vista comprimida.**
3. Haz doble clic en `SUBIR_A_GITHUB_E25.bat`. Autoriza el acceso a GitHub cuando lo solicite. El script crea `Zelda-Min-Cap-DSi` como **repositorio privado**, sube los archivos y abre la compilación en el navegador.
4. En GitHub, entra a **Actions → Build Zelda Minish Cap DSi E25 → último run**.
5. Cuando los dos trabajos salgan en verde, en **Artifacts** descarga `Zelda-Min-Cap-DSi-E25-NDS`. Adentro viene `tmc-dsi-engine-e25.nds`.
6. Si sale rojo, descarga `E25-Build-Log` del mismo run y comparte el registro para corregir la falla. Un archivo `.nds` **solo existe si la compilación terminó correctamente**.

## Para probar en DSi / melonDS

La ROM europea sin modificar que ya tienes debe estar en la SD FAT accesible como `tmc-dsi/baserom_eu.gba`. **No subas la ROM a GitHub**. Abre el `.nds` y pulsa START/A en el título. E25 busca mostrar una vista de la primera habitación en la pantalla superior; no es jugable todavía. El audio está desactivado por ahora.

## Alcance de las pruebas

El renderizador E25 pasa su prueba local de framebuffer en una PC Linux sin ROM. **Compilar para ARM9 y probar en hardware real siguen pendientes hasta que el workflow de GitHub Actions se complete.** La imagen de Docker `devkitpro/devkitarm:20260610` existe, pero eso por sí solo no demuestra que el código enlace o arranque.
