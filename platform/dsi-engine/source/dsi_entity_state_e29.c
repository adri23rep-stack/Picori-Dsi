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

/* E29.8 ARM9 telemetry adapter for PC_PORT instrumentation embedded in
 * the literal original AppendEntityToList body. This does NOT allocate or
 * modify entities, list links, priorities or any gameplay behavior.
 * Host tests provide their own logger; DSi records the actual call. */
#ifdef ARM9
volatile unsigned gDsiE29OriginalListAppends;
void Port_LogListOp(const char* operation, void* entity, unsigned kind,
                    unsigned id, unsigned listIndex, void* list) {
    (void)operation; (void)entity; (void)kind; (void)id;
    (void)listIndex; (void)list;
    ++gDsiE29OriginalListAppends;
}
#endif
