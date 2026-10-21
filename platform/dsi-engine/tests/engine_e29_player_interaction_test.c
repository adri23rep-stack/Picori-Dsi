/* E29.6: test genuine ResetPossibleInteraction against native RAM. */
#include "player.h"
#include "common.h"
#include "dsi_player_state_e29.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern const InteractableObject gNoInteraction;
void MemClear(void* dest, u32 size) { memset(dest, 0, size); }
int main(void) {
    memset(&gPossibleInteraction, 0xA5, sizeof gPossibleInteraction);
    memset(gActiveItems, 0xA5, sizeof gActiveItems);
    assert(!DsiPlayerState_Ready());
    DsiPlayerState_Reset();
    assert(DsiPlayerState_Ready());
    assert(gPossibleInteraction.currentObject == &gNoInteraction);
    assert(gPossibleInteraction.currentObject->type == INTERACTION_NULL);
    assert(gPossibleInteraction.currentObject->entity == NULL);
    assert(gPossibleInteraction.candidates[0].entity == NULL);
    assert(gActiveItems[0].priority == 0);
    gPossibleInteraction.candidates[4].kinstoneId = 3;
    assert(!DsiPlayerState_Ready());
    ResetPossibleInteraction();
    assert(DsiPlayerState_Ready());
    gActiveItems[1].priority = 4;
    assert(!DsiPlayerState_Ready());
    DsiPlayerState_Reset();
    assert(DsiPlayerState_Ready());
    puts("PASS E29.6: original ResetPossibleInteraction + native item RAM");
    return 0;
}
