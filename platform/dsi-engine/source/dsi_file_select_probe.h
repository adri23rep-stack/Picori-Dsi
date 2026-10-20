#ifndef DSI_FILE_SELECT_PROBE_H
#define DSI_FILE_SELECT_PROBE_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    unsigned prepared_slots;
    unsigned options_tested;
    unsigned intro_loads;
    unsigned language_loads;
    unsigned language_gfx_bytes;
    unsigned loaded_gfx_bytes;
    unsigned loaded_pal_bytes;
    unsigned errors;
} DsiFileSelectProbeStats;
extern DsiFileSelectProbeStats gDsiFileSelectProbeStats;
/* Invoke only when the original scheduler has reached TASK_FILE_SELECT.
 * Runs two unmodified, bounded original helpers; does NOT invoke
 * FileSelectTask or access any disk save data. */
bool DsiFileSelectProbe_Prepare(void);
/* ROM-only group preloading and original options helper; NO save I/O or UI. */
bool DsiFileSelectProbe_LoadIntroAssets(void);
bool DsiFileSelectProbe_TestOptions(void);
/* E16: load the two genuine graphics groups requested on first file-select
 * tick when current language differs from the menu cache. Still no save I/O. */
bool DsiFileSelectProbe_LoadLanguageAssets(void);
#endif
