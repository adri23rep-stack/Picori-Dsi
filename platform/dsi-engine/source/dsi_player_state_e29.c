/* E29.6: allocate genuine GBA player-interaction/item globals in DSi RAM.
 * ResetPossibleInteraction() itself is linked from ORIGINAL src/playerUtils.c.
 */
#include "dsi_player_state_e29.h"
#include "player.h"
#include <string.h>
PossibleInteraction gPossibleInteraction;
ItemBehavior gActiveItems[MAX_ACTIVE_ITEMS];
extern const InteractableObject gNoInteraction;
void DsiPlayerState_Reset(void) {
    memset(gActiveItems, 0, sizeof gActiveItems);
    ResetPossibleInteraction();
}
bool DsiPlayerState_Ready(void) {
    if (gPossibleInteraction.currentObject != (InteractableObject*)&gNoInteraction ||
        gPossibleInteraction.isUpdated || gPossibleInteraction.unused ||
        gPossibleInteraction.kinstoneId || gPossibleInteraction.currentIndex)
        return false;
    for (unsigned i = 0; i < 0x20u; ++i) {
        const InteractableObject *obj = &gPossibleInteraction.candidates[i];
        if (obj->ignoreLayer || obj->type || obj->interactDirections ||
            obj->kinstoneId || obj->customHitbox || obj->entity)
            return false;
    }
    const unsigned char *items = (const unsigned char*)gActiveItems;
    for (unsigned i = 0; i < sizeof gActiveItems; ++i)
        if (items[i]) return false;
    return true;
}
