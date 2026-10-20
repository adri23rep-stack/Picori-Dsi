/* E14 file-select *in-memory* bridge. Not an actual file-select UI.
 * Original src/fileselect.c supplies SetFileSelectState/ResetSaveFile.
 * Never read, create, truncate or write persistent saves in this stage. */
#include "dsi_file_select_probe.h"
#include "fileselect.h"
#include "main.h"
#include "screen.h"
#include "vram.h"
#include "dsi_title_assets.h"
#include "dsi_gba_bus.h"
#include "fade.h"
#include "port_tts.h"
#include <string.h>

/* File-select overlays this 32 KiB work buffer with save-slot data.
 * The GBA engine shares this buffer between several mutually exclusive
 * tasks. E14 allocates an independent DSi backing store for this task. */
u16 gMapDataBottomSpecial[0x4000] __attribute__((aligned(4)));
DsiFileSelectProbeStats gDsiFileSelectProbeStats;
/* File select original LoadOptionsFromSave() marks these palettes dirty.
 * Keep a real symbol while the full palette manager is not linked. */
u32 gUsedPalettes;
_Static_assert(sizeof(struct_02019EE0) <= sizeof(gMapDataBottomSpecial),
               "File select slots exceed isolated DSi RAM backing");

/* The existing DSi bridge already supplies the audio/TTS no-op. */
/* fileselect.c deliberately leaves these original functions non-static. */
extern void SetFileSelectState(int mode);
extern void ResetSaveFile(u32 save_idx);
extern void LoadOptionsFromSave(u32 idx);
extern void sub_080503A8(u32 group);
extern void __wrap_LoadPaletteGroup(u32 group);

bool DsiFileSelectProbe_Prepare(void) {
    if (gMain.task != TASK_FILE_SELECT) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    /* Make this probe idempotent and read-only with respect to saved games.
     * This array is shadow RAM and has no backing FAT filename. */
    memset(gMapDataBottomSpecial, 0, sizeof(gMapDataBottomSpecial));
    /* A fresh file-select visit must not inherit group counters from a
     * previous session, or the language/resource guard will false-fail. */
    memset(&gDsiFileSelectProbeStats, 0, sizeof(gDsiFileSelectProbeStats));
    gFileSelectState.unk3 = 7; /* original HandleFileScreenEnter() sentinel */
    SetFileSelectState(0); /* original STATE_NONE; clears original BG maps */
    if (gUI.state != 0) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    for (u32 i = 0; i < NUM_SAVE_SLOTS; ++i) {
        ResetSaveFile(i); /* original defaults, RAM only */
        const SaveFile *save = &gFileSelectState.saves[i];
        if (gFileSelectState.saveStatus[i] != 0 ||
            save->msg_speed != 1 || save->brightness != 1 ||
            save->stats.health != 24 || save->stats.maxHealth != 24) {
            ++gDsiFileSelectProbeStats.errors;
            return false;
        }
        ++gDsiFileSelectProbeStats.prepared_slots;
    }
    return true;
}

/* E15 restricted slice of ORIGINAL HandleFileScreenEnter: after file-select
 * slots have been prepared, invoke its existing graphics helper and load its
 * palette group.  Never run the unported handler, create entities, access
 * save files, or synthesize a replacement file-select UI. */
bool DsiFileSelectProbe_LoadIntroAssets(void) {
    if (gMain.task != TASK_FILE_SELECT ||
        gDsiFileSelectProbeStats.prepared_slots != NUM_SAVE_SLOTS ||
        !DsiTitleAssets_Active()) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    unsigned errors = gDsiTitleAssetStats.errors;
    unsigned gfx = gDsiTitleAssetStats.gfx_groups;
    unsigned palettes = gDsiTitleAssetStats.palette_groups;
    uint32_t bytes_gfx = gDsiTitleAssetStats.bytes_gfx;
    uint32_t bytes_pal = gDsiTitleAssetStats.bytes_pal;

    /* These group IDs are taken from the unmodified HandleFileScreenEnter().
     * sub_080503A8 is ORIGINAL fileselect.c, not rewritten here. */
    sub_080503A8(5);
    __wrap_LoadPaletteGroup(9);
    if (gDsiTitleAssetStats.errors != errors ||
        gDsiTitleAssetStats.gfx_groups != gfx + 1 ||
        gDsiTitleAssetStats.palette_groups != palettes + 1 ||
        gDsiTitleAssetStats.bytes_gfx <= bytes_gfx ||
        gDsiTitleAssetStats.bytes_pal <= bytes_pal) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    gDsiFileSelectProbeStats.loaded_gfx_bytes = gDsiTitleAssetStats.bytes_gfx - bytes_gfx;
    gDsiFileSelectProbeStats.loaded_pal_bytes = gDsiTitleAssetStats.bytes_pal - bytes_pal;
    ++gDsiFileSelectProbeStats.intro_loads;
    return true;
}

/* Original preferences conversion only. The live save-selection handler has
 * extensive persistence/repair code and is NEVER invoked by this bridge. */
bool DsiFileSelectProbe_TestOptions(void) {
    if (gMain.task != TASK_FILE_SELECT ||
        gDsiFileSelectProbeStats.prepared_slots != NUM_SAVE_SLOTS) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    for (u32 i = 0; i < NUM_SAVE_SLOTS; ++i) {
        gUsedPalettes = 0;
        LoadOptionsFromSave(i); /* ORIGINAL fileselect.c */
        if (gSaveHeader->msg_speed != gFileSelectState.saves[i].msg_speed ||
            gSaveHeader->brightness != gFileSelectState.saves[i].brightness ||
            gUsedPalettes != 0xffffffffu) {
            ++gDsiFileSelectProbeStats.errors;
            return false;
        }
        ++gDsiFileSelectProbeStats.options_tested;
    }
    gUsedPalettes = 0;
    LoadOptionsFromSave(NUM_SAVE_SLOTS); /* original choose-language defaults */
    if (gSaveHeader->msg_speed != 1 || gSaveHeader->brightness != 1 ||
        gUsedPalettes != 0xffffffffu) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    ++gDsiFileSelectProbeStats.options_tested;
    return true;
}

/* This is the read-only graphics tail of ORIGINAL FileSelectTask():
 * if (gFileSelectState.unk3 != gSaveHeader->language) { ... 6; ... 15; }
 * No handler dispatch, entity spawning, save file loading or persistence. */
bool DsiFileSelectProbe_LoadLanguageAssets(void) {
    if (gMain.task != TASK_FILE_SELECT ||
        gDsiFileSelectProbeStats.prepared_slots != NUM_SAVE_SLOTS ||
        gDsiFileSelectProbeStats.intro_loads != 1 ||
        !DsiTitleAssets_Active() || gDsiTitleAssetStats.errors != 0) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    if (gFileSelectState.unk3 == gSaveHeader->language) return true;
    unsigned before_groups = gDsiTitleAssetStats.gfx_groups;
    uint32_t before_bytes = gDsiTitleAssetStats.bytes_gfx;
    /* Actual helper is unchanged src/fileselect.c. Both groups are read
     * from the European ROM tables, not duplicated into this adapter. */
    sub_080503A8(6);
    sub_080503A8(15);
    if (gDsiTitleAssetStats.errors != 0 ||
        gDsiTitleAssetStats.gfx_groups != before_groups + 2) {
        ++gDsiFileSelectProbeStats.errors;
        return false;
    }
    gDsiFileSelectProbeStats.language_gfx_bytes =
        gDsiTitleAssetStats.bytes_gfx - before_bytes;
    ++gDsiFileSelectProbeStats.language_loads;
    gFileSelectState.unk3 = gSaveHeader->language;
    return true;
}
