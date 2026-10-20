/* E17: bounded ROM window for the ORIGINAL file-select palette animation.
 * sub_0805066C in src/fileselect.c adds offsets relative to
 * gGlobalGfxAndPalettes; staging just its referenced span preserves those
 * accesses without allocating the complete 16-MiB ROM in DSi memory.
 * No persistent save APIs are called by this module. */
#include "dsi_file_select_palette.h"
#include "dsi_gba_bus.h"
#include "fileselect.h"
#include "main.h"
#include <string.h>

#define EU_GFX_BASE 0x085A23D0u
#define FILE_SELECT_WINDOW 0x12000u
#define STAGE_CHUNK 4096u

static u8 sFileSelectPaletteWindow[FILE_SELECT_WINDOW] __attribute__((aligned(4)));
const u8* gGlobalGfxAndPalettes = NULL;
u16 gPaletteBuffer[512] __attribute__((aligned(4)));

extern void DsiOriginalFileSelectPaletteTick(void);
static bool sReady;
unsigned gDsiFileSelectPaletteTicks;

void DsiFileSelectPalette_Reset(void) {
    sReady = false;
    gGlobalGfxAndPalettes = NULL;
    gDsiFileSelectPaletteTicks = 0;
    memset(gPaletteBuffer, 0, sizeof(gPaletteBuffer));
}

bool DsiFileSelectPalette_Initialize(void) {
    if (gMain.task != TASK_FILE_SELECT) return false;
    DsiFileSelectPalette_Reset();
    /* ROM pages are small; a 4-KiB transfer fits the existing reader. */
    for (u32 offset = 0; offset < FILE_SELECT_WINDOW; offset += STAGE_CHUNK) {
        if (!DsiGbaBus_Read(EU_GFX_BASE + offset,
                            sFileSelectPaletteWindow + offset, STAGE_CHUNK)) {
            DsiFileSelectPalette_Reset();
            return false;
        }
    }
    gGlobalGfxAndPalettes = sFileSelectPaletteWindow;
    sReady = true;
    return true;
}

bool DsiFileSelectPalette_Step(void) {
    if (!sReady || !gGlobalGfxAndPalettes || gMain.task != TASK_FILE_SELECT) return false;
    /* This is the unmodified original color cycle. The game's own RNG,
     * timer, palette indices and language-dependent offsets are retained. */
    DsiOriginalFileSelectPaletteTick();
    ++gDsiFileSelectPaletteTicks;
    return true;
}

bool DsiFileSelectPalette_Ready(void) { return sReady; }

/* PC_PORT's inline DMA shim needs these two hooks. During the bounded E17
 * probe only this staged ROM window is a valid palette copy source. Neither
 * a GBA-address-like raw pointer nor an HBlank DMA request is accepted. */
const void *port_resolve_copy_src(const void *src, u32 size) {
    uintptr_t lo = (uintptr_t)sFileSelectPaletteWindow;
    uintptr_t p = (uintptr_t)src;
    /* An optional E19 host/native bridge may whitelist specific native RAM
     * objects. E3-E18 continue to accept ONLY the staged palette window. */
    extern bool DsiGameTransition_AllowNativeCopy(const void*, u32)
        __attribute__((weak));
    if (DsiGameTransition_AllowNativeCopy &&
        DsiGameTransition_AllowNativeCopy(src, size)) return src;
    if (!sReady || p < lo || p - lo > FILE_SELECT_WINDOW ||
        size > FILE_SELECT_WINDOW - (p - lo)) return NULL;
    return src;
}
void port_hdma_register(int channel, const void *src, void *dest,
                        uint16_t control, uint16_t count) {
    (void)channel; (void)src; (void)dest; (void)control; (void)count;
}
