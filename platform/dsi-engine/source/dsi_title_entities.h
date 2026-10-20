#ifndef DSI_TITLE_ENTITIES_H
#define DSI_TITLE_ENTITIES_H
/* Minimal bounded DSi transport for the ORIGINAL titleScreenObject.c module.
 * Not the complete object/entity engine. Does not synthesize gameplay rules. */
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    unsigned created, alive, updated, errors, unsupported;
    unsigned root_updates, child_updates;
    unsigned subtype_counts[5];
} DsiTitleEntityStats;
extern DsiTitleEntityStats gDsiTitleEntityStats;
void DsiTitleEntities_Enable(bool enabled);
void DsiTitleEntities_Clear(void);
void DsiTitleEntities_Update(void);
void DsiTitleEntities_Draw(void);
/* Preserve original engine signatures for linker hooks, implemented in .c */
#endif
