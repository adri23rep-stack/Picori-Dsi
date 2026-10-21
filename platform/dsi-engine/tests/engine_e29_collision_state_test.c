/* E29.5: original src/collision.c RegisterPlayerHitbox extracted at build.
 * Host MemClear is a test boundary; there is no fake collision logic. */
#include "collision.h"
#include "player.h"
#include "common.h"
#include "dsi_collision_state_e29.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
extern LinkedList2 gUnk_03003C70[16];
PlayerEntity gPlayerEntity;
void MemClear(void *p, u32 n) { memset(p,0,n); }
void RegisterPlayerHitbox(void);
int main(void) {
    DsiCollisionState_Reset();
    assert(DsiCollisionState_EmptyValid());
    assert(!DsiCollisionState_PlayerRegistered());
    memset(&gPlayerEntity,0x5A,sizeof gPlayerEntity);
    memset(gUnk_03003C70,0xA5,sizeof gUnk_03003C70);
    RegisterPlayerHitbox();
    assert(DsiCollisionState_PlayerRegistered());
    assert(gUnk_03003C70[0].node == (void*)&gPlayerEntity.base);
    assert(gUnk_03003C70[0].flags == 0);
    for (unsigned i=1; i<16; ++i)
        assert(!gUnk_03003C70[i].first && !gUnk_03003C70[i].last &&
               !gUnk_03003C70[i].node && !gUnk_03003C70[i].flags);
    RegisterPlayerHitbox();
    assert(DsiCollisionState_PlayerRegistered());
    DsiCollisionState_Reset();
    assert(DsiCollisionState_EmptyValid());
    puts("PASS E29.5: genuine RegisterPlayerHitbox backed by native RAM");
    return 0;
}
