#include "dsi_animation_e45.h"
#include "dsi_animation_metadata_e45.h"
#include "dsi_gba_bus.h"
#include "functions.h"
#include "asm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
GfxSlotList gGFXSlots;
static unsigned fault;
static FILE* rom;
#ifdef E46_UPLOAD_TEST
#include "color.h"
#include "dsi_sprite_upload_e46.h"
struct_gUnk_020000C0 gUnk_020000C0[48];
static u8 objVram[0x8000];
static unsigned writes;
void MemClear(void* p,u32 bytes) {memset(p,0,bytes);}
void sub_080ADD70(void);
void sub_080ADE74(u32 index);
bool DsiGbaBus_IsWritable(uint32_t a,size_t n) {
    return a>=0x06010000&&a<=0x06018000&&n<=0x06018000-a;
}
bool DsiGbaBus_Write(uint32_t a,const void* source,size_t n) {
    if(!DsiGbaBus_IsWritable(a,n))return false;
    memcpy(objVram+a-0x06010000,source,n);writes++;return true;
}
#endif
bool DsiGbaBus_Read(uint32_t a,void* dest,size_t n) {
    if(rom) {
        return a>=0x08000000&&a-0x08000000<=0x1000000&&n<=0x1000000-(a-0x08000000)&&
               fseek(rom,a-0x08000000,SEEK_SET)==0&&fread(dest,1,n,rom)==n;
    }
    u8* b=dest;u32 value=0;
#ifdef E46_UPLOAD_TEST
    const u8 compressed[]={0x10,4,0,0,0,1,2,3,4};
    if(a>=0x08f00000&&a<=0x08f00000+sizeof compressed&&n<=sizeof compressed-(a-0x08f00000)) {
        memcpy(dest,compressed+a-0x08f00000,n);return true;
    }
#endif
    if(fault==1)return false;
    if(a==E45_ANIM_ROM&&n==E45_ANIM_BYTES) {
        memset(dest,0,n);
        const u8 sequence[]={1,2,0x10,1,2,3,0x20,0,3,1,0x30,0x80,3};
        memcpy(dest,sequence,sizeof sequence);return true;
    }
    if(a==E45_GFX_ROM&&n==E45_GFX_BYTES) {
        if(fault==6)return false;
        for(unsigned i=0;i<n;i++)b[i]=i;
        return true;
    }
    for(unsigned i=0;i<E45_SPRITES;i++) {
        const E45Metadata* m=&e45Metadata[i];
        if(n==4&&a>=0x08002a5c+m->sprite*16&&a<0x08002a5c+m->sprite*16+16) {
            unsigned field=(a-(0x08002a5c+m->sprite*16))/4;
            const u32 fields[]={m->animations,m->frames,E45_GFX_ROM,0};
            value=fields[field];if(fault==2)value++;goto word;
        }
        if(n==4&&a>=m->animations&&a<=m->animations+m->animCount*4) {
            value=a==m->animations+m->animCount*4?0:E45_ANIM_ROM;
            if(fault==3&&value==0)value=1;
            if(fault==4&&value)value=E45_ANIM_ROM+E45_ANIM_BYTES-3;
            goto word;
        }
        if(n==4&&a>=m->frames&&a<m->frames+m->frameCount*4) {
            unsigned frame=(a-m->frames)/4;
            value=2|(frame<<18);if(fault==5)value=0xffff00ff;goto word;
        }
    }
    return false;
word:
    for(unsigned i=0;i<4;i++)b[i]=value>>(i*8);
    return true;
}
static void synthetic(void) {
    assert(!DsiAnimationE45_Ready());
    for(fault=1;fault<=6;fault++) {
        assert(!DsiAnimationE45_Prepare());assert(!DsiAnimationE45_Ready());
    }
    fault=0;assert(DsiAnimationE45_Prepare());
    assert(!DsiAnimationE45_CanInitialize(10,0));
    assert(!DsiAnimationE45_CanInitialize(1,229));
    Entity e={0};e.spriteIndex=1;e.spriteAnimation[0]=2;
    gGFXSlots.slots[2].status=GFX_SLOT_PALETTE;
    InitAnimationForceUpdate(&e,2);
    assert(e.frameIndex==1&&e.frameDuration==2&&e.frameSpriteSettings==0x10&&e.frame==1);
    assert(gGFXSlots.slots[2].paletteIndex==2&&gGFXSlots.slots[2].vramStatus==GFX_VRAM_3);
    const SpritePtr* s=DsiAnimationE45_Sprite(1);
    assert(gGFXSlots.slots[2].palettePointer==(u8*)s->ptr+128);
    gGFXSlots.slots[2].vramStatus=GFX_VRAM_1;
    UpdateAnimationSingleFrame(&e);assert(e.frameIndex==1&&e.frameDuration==1);
    assert(gGFXSlots.slots[2].vramStatus==GFX_VRAM_1);
    UpdateAnimationSingleFrame(&e);assert(e.frameIndex==2&&e.frameDuration==3);
    assert(gGFXSlots.slots[2].palettePointer==(u8*)s->ptr+256);
    assert(gGFXSlots.slots[2].vramStatus==GFX_VRAM_3);
    UpdateAnimationVariableFrames(&e,4);assert(e.frameIndex==1&&e.frameDuration==2);
    /* Original GetNextFrame changes animation only, not the GFX slot. */
    GfxSlot saved=gGFXSlots.slots[2];GetNextFrame(&e);
    assert(e.frameDuration==1&&!memcmp(&saved,&gGFXSlots.slots[2],sizeof saved));
#ifdef E46_UPLOAD_TEST
    memset(objVram,0xa5,sizeof objVram);gGFXSlots.unk0=1;
    InitAnimationForceUpdate(&e,2);
    sub_080ADD70();
    assert(gGFXSlots.slots[2].vramStatus==GFX_VRAM_1);
    assert(!memcmp(objVram+0x2c00,(u8*)s->ptr+128,64));
    assert(objVram[0x2bff]==0xa5&&objVram[0x2c40]==0xa5);
    unsigned previous=writes;sub_080ADD70();assert(writes==previous);
    UpdateAnimationSingleFrame(&e);sub_080ADD70();assert(writes==previous);
    UpdateAnimationSingleFrame(&e);sub_080ADD70();assert(writes==previous+1);
    assert(!memcmp(objVram+0x2c00,(u8*)s->ptr+256,64));
    gGFXSlots.unk_3=1;sub_080ADD70();
    assert(!gGFXSlots.unk_3&&writes==previous+2);
    GfxSlot* compressedSlot=&gGFXSlots.slots[3];
    compressedSlot->status=GFX_SLOT_RESERVED;compressedSlot->vramStatus=GFX_VRAM_3;
    compressedSlot->paletteIndex=0xffff;compressedSlot->palettePointer=(void*)0x08f00000;
    sub_080ADE74(3);
    assert(!memcmp(objVram+0x2e00,"\1\2\3\4",4));
    puts("E46 original upload: dirty/clean frames, forced refresh, OBJ bounds and ROM LZ PASS");
#endif
    /* Invalid ROM-derived native access terminates instead of reading a
       foreign pointer, crossing the stream buffer or inventing a frame. */
    unsigned checks=3;
#ifdef E46_UPLOAD_TEST
    checks=8;
#endif
    for(unsigned check=0;check<checks;check++) {
        pid_t pid=fork();assert(pid>=0);
        if(!pid) {
            if(check==0)DsiAnimationE45_Frame(8,200);
            if(check==1)DsiAnimationE45_LoopTarget((u8*)DsiAnimationE45_Data(1,0),255);
            if(check==2)DsiAnimationE45_RequireBytes(&e,4);
#ifdef E46_UPLOAD_TEST
            if(check==3)DsiSpriteUploadE46_ValidateSlot(MAX_GFX_SLOTS);
            if(check==4) {gGFXSlots.slots[43].paletteIndex=17;DsiSpriteUploadE46_ValidateSlot(43);}
            if(check==5)DsiSpriteUploadE46_Copy((uintptr_t)&e,0x06010000,4);
            if(check==6)DsiSpriteUploadE46_Copy((uintptr_t)s->ptr,0x06000000,4);
            if(check==7)DsiSpriteUploadE46_Lz((void*)0x08f00001,(void*)0x06010000);
#endif
            _exit(0);
        }
        int status;assert(waitpid(pid,&status,0)==pid);
        assert(WIFSIGNALED(status)&&WTERMSIG(status)==SIGABRT);
    }
    puts("E45 original animation: transport failures, timing, loops, GFX dirty slots and bounds PASS");
}
static u32 read32(u32 a) {
    u8 b[4];assert(DsiGbaBus_Read(a,b,4));
    return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);
}
static void real_rom(const char* path) {
    rom=fopen(path,"rb");assert(rom&&DsiAnimationE45_Prepare());
    /* Replay the original sleep/wake and initial standing/walking streams,
       comparing every loaded frame byte against independent ROM reads. */
    const unsigned cases[][2]={{1,2},{1,4},{1,5},{1,6},{4,0},{4,4},{7,0x36},{7,0x3a},{7,0x37},{7,0x3b}};
    unsigned ticks=0;
    for(unsigned i=0;i<sizeof cases/sizeof *cases;i++) {
        Entity e={0};e.spriteIndex=cases[i][0];e.spriteAnimation[0]=2;
        u32 table=read32(0x08002a5c+e.spriteIndex*16);
        u32 address=read32(table+cases[i][1]*4);
        u8 frame[4];assert(DsiGbaBus_Read(address,frame,4));
        InitAnimationForceUpdate(&e,cases[i][1]);
        const u8* nativeStart=DsiAnimationE45_Data(e.spriteIndex,cases[i][1]);
        u32 next=address+4;
        if(frame[3]&0x80){u8 back;assert(DsiGbaBus_Read(next,&back,1));next-=back*4;}
        assert(e.frameIndex==frame[0]&&e.frameDuration==frame[1]&&
               e.frameSpriteSettings==frame[2]&&e.frame==frame[3]);
        bool waking=cases[i][0]==7&&(cases[i][1]==0x37||cases[i][1]==0x3b);
        bool wakeEvent=false,wakeDone=false;
        for(unsigned tick=0;tick<512;tick++) {
            /* PlayerSleep consumes ANIM_DONE and switches actions. Do not
               keep replaying a completed wake animation with duration zero. */
            if(waking&&(frame[3]&0x80)) {wakeDone=true;break;}
            /* These replay cases have positive durations; fail if the source
               changes instead of silently approximating zero-duration logic. */
            assert(frame[1]>0);
            if(--frame[1]==0) {
                assert(DsiGbaBus_Read(next,frame,4));next+=4;
                if(frame[3]&0x80){u8 back;assert(DsiGbaBus_Read(next,&back,1));next-=back*4;}
            }
            UpdateAnimationSingleFrame(&e);
            assert(e.frameIndex==frame[0]&&e.frameDuration==frame[1]&&
                   e.frameSpriteSettings==frame[2]&&e.frame==frame[3]);
            if(waking&&e.frame==1)wakeEvent=true;
            assert((intptr_t)e.animPtr-(intptr_t)nativeStart==(int64_t)next-address);
            SpriteFrame* f=DsiAnimationE45_Frame(e.spriteIndex,e.frameIndex);
            assert(gGFXSlots.slots[2].paletteIndex==f->numTiles);
            assert(gGFXSlots.slots[2].palettePointer==(u8*)DsiAnimationE45_Sprite(e.spriteIndex)->ptr+f->firstTileIndex*32);
#ifdef E46_UPLOAD_TEST
            gGFXSlots.unk0=1;sub_080ADD70();
            u8 originalTiles[8192];size_t bytes=f->numTiles*32;
            assert(bytes<=sizeof originalTiles);
            assert(DsiGbaBus_Read(E45_GFX_ROM+f->firstTileIndex*32,originalTiles,bytes));
            assert(!memcmp(objVram+0x2c00,originalTiles,bytes));
            assert(gGFXSlots.slots[2].vramStatus==GFX_VRAM_1);
#endif
            ticks++;
        }
        if(waking)assert(wakeDone&&wakeEvent);
    }
    fclose(rom);printf("E45 EU ROM: %u original Link animation ticks exactly match ROM records\n",ticks);
#ifdef E46_UPLOAD_TEST
    printf("E46 EU ROM: %u original frame uploads match ROM tile bytes\n",ticks);
#endif
}
int main(int argc,char** argv) {
    if(argc==2) {gGFXSlots.slots[2].status=GFX_SLOT_PALETTE;real_rom(argv[1]);}
    else synthetic();
}
