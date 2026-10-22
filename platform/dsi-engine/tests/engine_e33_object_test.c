/* E33 host regression: authentic BedCover and source-linked spawn/position. */
#include "dsi_room_object_tick_e33.h"
#include "dsi_room_spawn_e31.h"
#include "entity.h"
#include "player.h"
#include "manager.h"
#include "object.h"
#include "room.h"
#include "main.h"
#include "game.h"
#include "message.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Main gMain;
Message gMessage;
static bool roomReady;
bool DsiRoomSpawnE31_Ready(void) { return roomReady; }
extern void* GetEmptyEntityByKind(u32 kind);
extern void RegisterRoomEntity(Entity*,const EntityData*);
static Entity* spawn(unsigned kind,unsigned id,unsigned form){
    Entity* e=GetEmptyEntityByKind(kind); assert(e);
    EntityData d={0};d.kind=kind;d.flags=0x0f;d.id=id;d.type=form;
    e->kind=kind;e->id=id;e->type=form;
    RegisterRoomEntity(e,&d);
    return e;
}
int main(void) {
    EraseAllEntities();
    DsiRoomObjectE33_Reset();
    gMain.task=TASK_GAME;gMain.state=GAMETASK_MAIN;
    gMain.substate=GAMEMAIN_CHANGEROOM;
    gRoomControls.area=0x22;gRoomControls.room=0x15;
    gPlayerEntity.base.kind=PLAYER;
    AppendEntityToList(&gPlayerEntity.base,1);
    Entity* bed=spawn(OBJECT,BED_COVER,1);
    bed->x.HALF.HI=100;bed->y.HALF.HI=80;bed->collisionLayer=1;
    for(unsigned i=0;i<3;i++)spawn(MANAGER,10+i,0);
    assert(gEntCount==2 && gManagerCount==3);
    assert(!DsiRoomObjectE33_Tick());
    assert(bed->action==0 && !DsiRoomObjectE33_Ticks());
    roomReady=true;
    assert(DsiRoomObjectE33_Tick());
    assert(DsiRoomObjectE33_Ready());
    assert(bed->action==1 && bed->spriteSettings.draw==1);
    assert(bed->frameIndex==1 && bed->spritePriority.b0==6);
    for(unsigned i=0;i<4;i++)assert(DsiRoomObjectE33_Tick());
    assert(DsiRoomObjectE33_Ticks()==5);
    assert(gEntCount==2 && gManagerCount==3);
    assert(DeepFindEntityByID(OBJECT,BED_COVER)==bed);
    Entity* child=CreateObjectWithParent(bed,BED_COVER,4,0);
    assert(child && child->parent==bed && child->x.WORD==bed->x.WORD);
    assert(child->y.WORD==bed->y.WORD && child->collisionLayer==bed->collisionLayer);
    assert(child->type==4 && gEntCount==3);
    /* Unsupported extra entity: do not keep running partial world updates. */
    assert(!DsiRoomObjectE33_Tick());
    assert(DsiRoomObjectE33_Errors()==1);
    EraseAllEntities();
    puts("E33 original BedCover: action0/action1, parent spawn, positioning, fail-closed PASS");
}
