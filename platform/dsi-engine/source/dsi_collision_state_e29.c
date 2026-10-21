/* E29.5 — Original player-hitbox globals in DSi native RAM.
 * RegisterPlayerHitbox() itself comes UNMODIFIED from src/collision.c.
 * No custom physics, hit detection, or game player is provided.
 */
#include "dsi_collision_state_e29.h"
#include "collision.h"
#include "player.h"
#include <string.h>
LinkedList2 gUnk_03003C70[16];
LinkedList2* gUnk_02018EA0;

void DsiCollisionState_Reset(void) {
    memset(gUnk_03003C70, 0, sizeof gUnk_03003C70);
    gUnk_02018EA0 = NULL;
}
bool DsiCollisionState_EmptyValid(void) {
    if (gUnk_02018EA0 != NULL) return false;
    for (unsigned i=0; i<16u; ++i) {
        if (gUnk_03003C70[i].last || gUnk_03003C70[i].first ||
            gUnk_03003C70[i].node || gUnk_03003C70[i].flags) return false;
    }
    return true;
}
bool DsiCollisionState_PlayerRegistered(void) {
    LinkedList2 *first = &gUnk_03003C70[0];
    return gUnk_02018EA0 == (LinkedList2*)(void*)&first->last &&
           first->last == &first->last &&
           first->first == &first->last &&
           first->node == (void*)&gPlayerEntity.base;
}
