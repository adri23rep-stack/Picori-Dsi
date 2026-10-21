#include "dsi_room_spawn_audit_e30.h"
#include "dsi_room_property_data_e30.h"
#include "room.h"
#include "entity.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
RoomControls gRoomControls;
u8 gEntCount, gManagerCount;
static bool srcReady;
static EntityData bank[2][5];
static unsigned counts[2]={3,2};
bool DsiRoomPropertyNativeE30_Ready(void) { return srcReady; }
const EntityData *DsiRoomPropertyDataE30_EntityBank(unsigned i) {
    return i<2 && srcReady ? bank[i] : NULL;
}
unsigned DsiRoomPropertyDataE30_Count(unsigned i) {
    return i<2 ? counts[i] : 0u;
}
int main(void) {
    DsiRoomSpawnAuditE30_Reset();
    assert(!DsiRoomSpawnAuditE30_Ready());
    srcReady=true; gEntCount=1;
    bank[0][0].kind=NPC; bank[0][0].flags=0x40;
    bank[0][1].kind=MANAGER; bank[0][1].flags=0x10;
    bank[0][2].kind=NPC; bank[0][2].flags=0x50;
    bank[0][3].kind=0xff;
    bank[1][0].kind=OBJECT; bank[1][0].flags=0x00;
    bank[1][1].kind=NPC; bank[1][1].flags=0x10;
    bank[1][2].kind=0xff;
    assert(DsiRoomSpawnAuditE30_Run());
    const DsiRoomSpawnAuditE30 *v=DsiRoomSpawnAuditE30_Get();
    assert(v && v->records==5 && v->scripted==1 &&
           v->managers==1 && v->duplicate_check==1 &&
           v->collision==3 && v->unfamiliar_kind==0);
    assert(!DsiRoomSpawnAuditE30_Run());
    gManagerCount=1; assert(!DsiRoomSpawnAuditE30_Ready());
    gManagerCount=0; assert(DsiRoomSpawnAuditE30_Ready());
    srcReady=false; assert(!DsiRoomSpawnAuditE30_Ready());
    DsiRoomSpawnAuditE30_Reset();
    counts[0]=4; /* fifth record not terminator -> fail closed */
    srcReady=true;
    assert(!DsiRoomSpawnAuditE30_Run());
    assert(DsiRoomSpawnAuditE30_Get()==NULL);
    puts("PASS E30.8: original EU room EntityData branch inventory and safe gating");
    return 0;
}
