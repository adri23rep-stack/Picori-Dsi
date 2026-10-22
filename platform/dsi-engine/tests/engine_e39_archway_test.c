/* Source-linked genuine ARCHWAY ID 0x4f != BED_COVER ID 0x5b.
 * Exercise original Archway action0 and idempotent action1, no fake AI. */
#include "dsi_room_archway_e39.h"
#include "dsi_room_spawn_e31.h"
#include "entity.h"
#include "object.h"
#include "area.h"
#include "player.h"
#include "game.h"
#include "main.h"
#include <assert.h>
#include <stdio.h>
Main gMain;
static bool ready;
bool DsiRoomSpawnE31_Ready(void){return ready;}
extern void* GetEmptyEntityByKind(u32);
extern void RegisterRoomEntity(Entity*,const EntityData*);
static Entity* spawn(unsigned kind,unsigned id,unsigned type){
 Entity* e=GetEmptyEntityByKind(kind);assert(e);
 EntityData d={0};d.kind=kind;d.flags=15;d.id=id;d.type=type;
 e->kind=kind;e->id=id;e->type=type;
 RegisterRoomEntity(e,&d);return e;
}
int main(void){
 assert(ARCHWAY==0x4f && BED_COVER==0x5b);
 EraseAllEntities();DsiRoomArchwayE39_Reset();
 gMain.task=TASK_GAME;gMain.state=GAMETASK_MAIN;
 gMain.substate=GAMEMAIN_CHANGEROOM;
 gRoomControls.area=0x22;gRoomControls.room=0x15;
 gPlayerEntity.base.kind=PLAYER;AppendEntityToList(&gPlayerEntity.base,1);
 Entity* arch=spawn(OBJECT,ARCHWAY,1);
 arch->type2=0;arch->x.HALF.HI=100;arch->y.HALF.HI=80;
 for(unsigned i=0;i<3;i++)spawn(MANAGER,10+i,0);
 assert(!DeepFindEntityByID(OBJECT,BED_COVER));
 assert(DeepFindEntityByID(OBJECT,ARCHWAY)==arch);
 assert(!DsiRoomArchwayE39_Tick());
 ready=true;
 assert(DsiRoomArchwayE39_Tick());
 assert(DsiRoomArchwayE39_Ready());
 assert(arch->action==1 && arch->spriteSettings.draw==1);
 assert(arch->collisionLayer==2 && arch->frameIndex==arch->type2);
 assert(DsiRoomArchwayE39_Tick());
 assert(gEntCount==2 && gManagerCount==3);
 assert(DsiRoomArchwayE39_Reason()==0);
 puts("E39 authentic ARCHWAY source action0/1, object ID mapping, priority PASS");
 EraseAllEntities();
}
