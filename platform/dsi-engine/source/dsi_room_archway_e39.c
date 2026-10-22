/* E39: Dispatch the ORIGINAL ARCHWAY object (ROM id 0x4F).
 * E33 incorrectly assumed 0x4F meant BED_COVER; BED_COVER is actually 0x5B.
 * Do not modify source EntityData or substitute a fake actor.
 * The exact src/object/archway.c::Archway body is linked separately. */
#include "dsi_room_archway_e39.h"
#include "dsi_room_spawn_e31.h"
#include "entity.h"
#include "object.h"
#include "player.h"
#include "main.h"
#include "game.h"
#include "room.h"
static Entity* sArch;
static unsigned sTicks,sReason;
void DsiRoomArchwayE39_Reset(void) { sArch=0; sTicks=sReason=0; }
unsigned DsiRoomArchwayE39_Ticks(void) {return sTicks;}
unsigned DsiRoomArchwayE39_Reason(void) {return sReason;}
bool DsiRoomArchwayE39_Ready(void) {
    return sTicks>0 && sReason==0 && sArch &&
       sArch->kind==OBJECT && sArch->id==ARCHWAY &&
       sArch->action==1 && sArch->spriteSettings.draw==1 &&
       sArch->frameIndex==sArch->type2 && sArch->collisionLayer==2;
}
bool DsiRoomArchwayE39_Tick(void) {
    if(!DsiRoomSpawnE31_Ready()) {sReason=1;return false;}
    if(gMain.task!=TASK_GAME || gMain.state!=GAMETASK_MAIN ||
       gMain.substate!=GAMEMAIN_CHANGEROOM ||
       gRoomControls.area!=0x22 || gRoomControls.room!=0x15 ||
       gEntCount!=2 || gManagerCount!=3 ||
       gPlayerEntity.base.kind!=PLAYER) {sReason=2;return false;}
    Entity* obj=DeepFindEntityByID(OBJECT,ARCHWAY);
    if(!obj) {sReason=3;return false;}
    if(obj->type!=1 || (obj->flags&(ENT_DELETED|ENT_SCRIPTED)) ||
       obj->action>1 || (sArch && sArch!=obj)) {sReason=4;return false;}
    sArch=obj;
    if(EntityDisabled(obj)) return DsiRoomArchwayE39_Ready();
    Archway(obj); /* UNMODIFIED original Archway() */
    if(obj->action!=1 || !obj->spriteSettings.draw ||
       obj->frameIndex!=obj->type2 || obj->collisionLayer!=2 ||
       gEntCount!=2 || gManagerCount!=3) {sReason=5;return false;}
    sReason=0;++sTicks;
    return true;
}
