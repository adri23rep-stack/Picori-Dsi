/* E40: prove unverified Archway compositor is quarantined while
 * source-correct ARCHWAY entity remains eligible for real update. */
#include "dsi_room_sprite_policy_e40.h"
#include "object.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    DsiRoomMetadata room={0};
    room.area=0x22; room.room=0x15;
    Entity arch={0};
    arch.kind=OBJECT;arch.id=ARCHWAY;
    arch.type=1;arch.type2=2;
    arch.action=1;arch.frameIndex=2;
    arch.spriteSettings.draw=1;
    assert(ARCHWAY==0x4f && BED_COVER==0x5b);
    assert(DsiRoomSpritePolicyE40_Archway(&room,&arch)==
       DSI_E40_ARCH_GFX_WAITING_FOR_NATIVE_SPRITE_PIPELINE);
    assert(!DsiRoomSpritePolicyE40_AllowPreview(&room,&arch));
    /* Action state is not mutated by suppressing corrupt presentation. */
    assert(arch.action==1 && arch.kind==OBJECT &&
           arch.id==ARCHWAY && arch.spriteSettings.draw);
    assert(DsiRoomSpritePolicyE40_Archway(&room,0)==DSI_E40_ARCH_GFX_INVALID);
    arch.id=BED_COVER;
    assert(DsiRoomSpritePolicyE40_Archway(&room,&arch)==DSI_E40_ARCH_GFX_INVALID);
    arch.id=ARCHWAY;
    room.area=0x23;
    assert(!DsiRoomSpritePolicyE40_AllowPreview(&room,&arch));
    puts("E40 arch graphics quarantine preserves original entity & no invented art PASS");
}
