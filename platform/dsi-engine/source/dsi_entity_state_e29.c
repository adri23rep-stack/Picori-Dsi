/* E29.4b — Genuine entity globals relocated from GBA EWRAM to DS RAM.
 * Native intrusive-list centinels match the original src/entity.c layout.
 * The actual AppendEntityToList() and priority operations are extracted
 * verbatim from the original source at build time.
 */
#include "dsi_entity_state_e29.h"
#include "entity.h"
#include <string.h>

LinkedList gEntityLists[9];
GenericEntity gAuxPlayerEntities[MAX_AUX_PLAYER_ENTITIES];
GenericEntity gEntities[MAX_ENTITIES];
u8 gEntCount;
u8 gManagerCount;

void DsiEntityState_Reset(void) {
    memset(gEntityLists, 0, sizeof gEntityLists);
    memset(gAuxPlayerEntities, 0, sizeof gAuxPlayerEntities);
    memset(gEntities, 0, sizeof gEntities);
    gEntCount = 0;
    gManagerCount = 0;
    /* The ORIGINAL AppendEntityToList writes through list->last->next.
     * first/last must therefore be the self-referential list sentinel
     * from the original GBA engine, never a null head. */
    for (unsigned i = 0; i < 9u; ++i) {
        Entity *sentinel = (Entity *)(void *)&gEntityLists[i];
        gEntityLists[i].first = sentinel;
        gEntityLists[i].last = sentinel;
    }
}

bool DsiEntityState_EmptyValid(void) {
    if (gEntCount != 0 || gManagerCount != 0) return false;
    for (unsigned i = 0; i < 9u; ++i) {
        const Entity *sentinel = (const Entity *)(const void *)&gEntityLists[i];
        if (gEntityLists[i].first != sentinel ||
            gEntityLists[i].last != sentinel) return false;
    }
    for (unsigned i = 0; i < MAX_ENTITIES; ++i) {
        if (gEntities[i].base.prev != NULL || gEntities[i].base.next != NULL)
            return false;
    }
    for (unsigned i = 0; i < MAX_AUX_PLAYER_ENTITIES; ++i) {
        if (gAuxPlayerEntities[i].base.prev != NULL ||
            gAuxPlayerEntities[i].base.next != NULL) return false;
    }
    return true;
}
