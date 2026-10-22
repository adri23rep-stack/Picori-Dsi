#include "entity.h"
#include "area.h"
#include "player.h"
#include "room.h"
#include "game.h"
#include "script.h"
#include "color.h"
#include "collision.h"
#include "common.h"
#include "enemy.h"
#include "port_entity_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
PlayerEntity gPlayerEntity;
PriorityHandler gPriorityHandler;
RoomTransition gRoomTransition;
RoomControls gRoomControls;
Area gArea;
OAMControls gOAMControls;
GfxSlotList gGFXSlots;
PossibleInteraction gPossibleInteraction;
extern UpdateContext gUpdateContext;
extern u8 gzHeap[4096];
extern void ClearAllDeletedEntities(void);
extern void ClearDeletedEntity(Entity*);
extern void DeleteManager(void*);
extern void DeleteEntity(Entity*);
extern void RegisterRoomEntity(Entity*,const EntityData*);
extern ScriptExecutionContext gScriptExecutionContextArray[32];
void MemClear(void* d,u32 n){memset(d,0,n);}
void MemCopy(const void* s,void* d,u32 n){memcpy(d,s,n);}
void Port_LogListOp(const char* o,void* e,unsigned k,unsigned i,unsigned l,void* p){(void)o;(void)e;(void)k;(void)i;(void)l;(void)p;}
static Entity* spawn(unsigned kind,unsigned id){
    extern void* GetEmptyEntityByKind(u32);
    Entity* e=GetEmptyEntityByKind(kind);assert(e);
    EntityData d={0};d.kind=kind;d.id=id;d.flags=0x0f;
    e->kind=kind;e->id=id;RegisterRoomEntity(e,&d);return e;
}
static void check_lists(void){
    unsigned ents=0,managers=0;
    for(unsigned l=0;l<9;l++){
        Entity* end=(Entity*)&gEntityLists[l];Entity* prev=end;unsigned cap=0;
        for(Entity* e=end->next;e!=end;e=e->next){assert(e&&++cap<110);assert(e->prev==prev);prev=e;if(e->kind==MANAGER)managers++;else ents++;}
        assert(end->prev==prev);
    }
    assert(ents==gEntCount&&managers==gManagerCount);
}
#ifndef E31_ROOM_TEST
int main(void){
    EraseAllEntities();InitScriptData();zMallocInit();
    Entity* e=spawn(NPC,4);Entity* next=spawn(NPC,5);
    static Script script[]={0};
    assert(StartCutscene(e,script));assert(Port_GetEntityScriptCtx(e)->scriptInstructionPointer==script);
    e->myHeap=zMalloc(24);e->hitbox=zMalloc(12);assert(e->myHeap&&e->hitbox);
    gPossibleInteraction.candidates[3].entity=e;gCarriedEntity.count=2;gCarriedEntity.unk_c[0]=e;gCarriedEntity.unk_c[1]=next;
    e->spriteAnimation[0]=2;gGFXSlots.slots[2].status=GFX_SLOT_GFX;gGFXSlots.slots[2].referenceCount=1;gGFXSlots.slots[2].slotCount=2;
    e->spriteAnimation[1]=3;gPaletteList[3]._0_0=3;gPaletteList[3]._0_4=1;gPaletteList[3]._1=1;
    gUpdateContext.current_entity=e;
    DeleteEntity(e);
    assert(!e->next&&(intptr_t)e->prev==-1);assert(!e->myHeap&&!e->hitbox);assert(!Port_GetEntityScriptCtx(e));
    assert(!gPossibleInteraction.candidates[3].entity&&gCarriedEntity.count==1&&gCarriedEntity.unk_c[0]==next);
    assert(!*(u16*)gzHeap);assert(gGFXSlots.slots[2].status==GFX_SLOT_UNLOADED);assert(gPaletteList[3]._0_0==1);
    assert(gUpdateContext.current_entity==(Entity*)&gEntityLists[7]);
    ClearAllDeletedEntities();assert(gEntCount==1);check_lists();
    Entity* manager=spawn(MANAGER,10);gArea.transitionManager=(void*)manager;gArea.onEnter=(void*)1;gArea.onExit=(void*)1;
    DeleteManager(manager);assert(!gArea.transitionManager&&!gArea.onEnter&&!gArea.onExit);check_lists();
    EraseAllEntities();assert(gEntCount==0&&gManagerCount==0);check_lists();
    /* Full pool: active current entity cannot be recycled; next eligible one can. */
    for(unsigned i=0;i<MAX_ENTITIES-1;i++)spawn(NPC,i);
    gEntities[0].base.flags=ENT_UNUSED1;gEntities[1].base.flags=ENT_UNUSED1;
    gUpdateContext.current_entity=&gEntities[0].base;
    assert(GetEmptyEntity()==&gEntities[1].base);assert(gEntCount==MAX_ENTITIES-2);
    assert(gEntities[0].base.next);check_lists();
    gUpdateContext.current_entity=NULL;EraseAllEntities();check_lists();
    /* All script slots use original allocation and can be freed/reused. */
    for(unsigned i=0;i<32;i++)assert(StartCutscene(spawn(NPC,i),script)==&gScriptExecutionContextArray[i]);
    assert(StartCutscene(spawn(NPC,33),script)==NULL);
    EraseAllEntities();assert(CreateScriptExecutionContext()==gScriptExecutionContextArray);check_lists();
    puts("E31 original lifecycle: allocation, deletion, recycling, resources, scripts and managers PASS");
}

#endif
