#include "common.h"
#include "structures.h"
#include "color.h"
#include "physics.h"
#include "vram.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
GfxSlotList gGFXSlots;
OAMControls gOAMControls;
LinkedList gEntityLists[9];
struct_gUnk_020000C0 gUnk_020000C0[48];
Palette gPaletteList[16];
void MemClear(void* p,u32 n) { memset(p,0,n); }
void ResetPalettes(void);
void sub_0806FD8C(void);
void UnloadOBJPalette2(Entity*);
void CleanUpGFXSlots(void);
static void reset(void) {
    ResetPalettes();sub_0806FD8C();
    memset(&gOAMControls,0,sizeof gOAMControls);
    for(unsigned i=0;i<9;i++) {
        gEntityLists[i].first=gEntityLists[i].last=(Entity*)&gEntityLists[i];
    }
}
static void attach(Entity* e,unsigned list) {
    Entity* sentinel=(Entity*)&gEntityLists[list];
    e->prev=sentinel;e->next=sentinel;
    gEntityLists[list].first=gEntityLists[list].last=e;
}
int main(void) {
    reset();
    /* Original PlayerInit's two resource allocations, without calling it. */
    Entity link={0};
    assert(sub_0806FDA0(&link)&&link.spriteAnimation[2]==1);
    for(unsigned p=0;p<4;p++)assert(gUnk_020000C0[1].unk_00[p].unk_01==255);
    assert(LoadSwapGFX(&link,1,2));
    assert(link.spriteAnimation[0]==2&&link.spriteVramOffset==0x160);
    assert(gGFXSlots.slots[2].status==GFX_SLOT_PALETTE);
    assert(gGFXSlots.slots[2].referenceCount==0x80);
    UnloadGFXSlots(&link);UnloadOBJPalette2(&link);
    assert(!link.spriteAnimation[0]&&!link.spriteAnimation[2]);
    assert(!gUnk_020000C0[1].unk_00[0].unk_00.unk0);
    assert(gGFXSlots.slots[2].referenceCount==0x80);
    /* Exhaust/reuse all 47 extra-sprite entries; index zero stays reserved. */
    Entity entities[48]={0};
    for(unsigned i=0;i<47;i++) {
        assert(sub_0806FDA0(&entities[i]));
        assert(entities[i].spriteAnimation[2]==i+1);
    }
    entities[47].spriteAnimation[2]=99;
    assert(!sub_0806FDA0(&entities[47]));
    assert(entities[47].spriteAnimation[2]==99);
    UnloadOBJPalette2(&entities[20]);
    assert(sub_0806FDA0(&entities[47])&&entities[47].spriteAnimation[2]==21);
    /* Dynamic slot exhaustion, failed requests, unload and contiguous reuse. */
    reset();memset(entities,0,sizeof entities);
    for(unsigned i=0;i<40;i++)assert(LoadSwapGFX(&entities[i],1,0));
    GfxSlotList full=gGFXSlots;Entity untouched=entities[40];
    assert(!LoadSwapGFX(&entities[40],1,0));
    assert(!memcmp(&full,&gGFXSlots,sizeof full));
    assert(!memcmp(&untouched,&entities[40],sizeof untouched));
    assert(!LoadSwapGFX(&entities[40],0,0));
    assert(!LoadSwapGFX(&entities[40],41,0));
    assert(!LoadSwapGFX(&entities[40],2,43));
    assert(!LoadSwapGFX(&entities[40],1,44));
    UnloadGFXSlots(&entities[10]);UnloadGFXSlots(&entities[11]);
    assert(LoadSwapGFX(&entities[40],2,0));
    assert(entities[40].spriteAnimation[0]==14);
    assert(gGFXSlots.slots[15].status==GFX_SLOT_FOLLOWER);
    UnloadGFXSlots(&entities[40]);
    assert(gGFXSlots.slots[14].status==GFX_SLOT_UNLOADED);
    assert(gGFXSlots.slots[15].status==GFX_SLOT_UNLOADED);
    /* Original compaction follows active lists, extra sprites and OAM. */
    reset();
    Entity active={0},suspended={0};
    assert(LoadSwapGFX(&active,2,10));
    assert(sub_0806FDA0(&active));assert(sub_0806FDA0(&suspended));
    attach(&active,1);
    gUnk_020000C0[active.spriteAnimation[2]].unk_00[0].unk_08.HALF_U.HI=0x1e3;
    gUnk_020000C0[suspended.spriteAnimation[2]].unk_00[0].unk_08.HALF_U.HI=0x1e3;
    gOAMControls.oam[0].tileNum=0x1e3;
    gGFXSlots.unk0=1;CleanUpGFXSlots();
    assert(active.spriteAnimation[0]==4&&active.spriteVramOffset==0x180);
    assert(gGFXSlots.slots[4].slotCount==2&&gGFXSlots.slots[5].status==GFX_SLOT_FOLLOWER);
    assert(gUnk_020000C0[active.spriteAnimation[2]].unk_00[0].unk_08.HALF_U.HI==0x183);
    assert(gUnk_020000C0[suspended.spriteAnimation[2]].unk_00[0].unk_08.HALF_U.HI==0x1e3);
    assert(gOAMControls.oam[0].tileNum==0x183&&gOAMControls.field_0x0==1);
    for(unsigned i=0;i<4;i++)assert(gGFXSlots.slots[i].status==GFX_SLOT_PALETTE);
    puts("E44 original sprite slots: Link init resources, exhaustion, reuse, compaction and cleanup PASS");
}
