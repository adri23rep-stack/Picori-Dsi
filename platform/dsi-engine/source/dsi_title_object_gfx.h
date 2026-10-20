#ifndef DSI_TITLE_OBJECT_GFX_H
#define DSI_TITLE_OBJECT_GFX_H
/* Bounded EU-title-only adapter for ORIGINAL ObjectDefinition entries.
 * Validates and loads the original fixed graphics & palette records into
 * the GBA shadow OBJ VRAM. Not the full GBA gfx-slot manager. */
#include <stdbool.h>
#include <stdint.h>
#include "entity.h"
typedef struct {
    unsigned gfx_loaded, gfx_reused, pal_loaded, pal_reused;
    unsigned gfx_bytes, pal_bytes, errors;
} DsiTitleObjectGfxStats;
extern DsiTitleObjectGfxStats gDsiTitleObjectGfxStats;
void DsiTitleObjectGfx_Reset(void);
bool DsiTitleObjectGfx_InitEntity(Entity *entity);
#endif
