/* Native stable storage for ORIGINAL Link animation/frames/tiles in EU ROM.
 * No animation timing, action selection or sprite frame is invented here. */
#include "dsi_animation_e45.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "dsi_animation_metadata_e45.h"
#include <stdlib.h>
#define EU_SPRITE_PTRS 0x08002a5cu
static bool sReady;
static u8 sAnimations[E45_ANIM_BYTES];
static u8* sTiles;
static SpriteFrame sFrames[E45_SPRITES][256];
static u32 sOffsets[E45_SPRITES][256];
static SpritePtr sSprites[E45_SPRITES];
static int find_sprite(u16 sprite) {
    for(unsigned i=0;i<E45_SPRITES;i++)if(e45Metadata[i].sprite==sprite)return i;
    return -1;
}
bool DsiAnimationE45_Ready(void) {return sReady;}
bool DsiAnimationE45_IsTileSpan(const void* pointer,size_t bytes) {
    uintptr_t p=(uintptr_t)pointer,base=(uintptr_t)sTiles;
    return sReady&&p>=base&&p-base<=E45_GFX_BYTES&&bytes<=E45_GFX_BYTES-(p-base);
}
bool DsiAnimationE45_CanInitialize(u16 sprite,u32 animation) {
    int slot=find_sprite(sprite);
    return sReady&&slot>=0&&animation<e45Metadata[slot].animCount;
}
bool DsiAnimationE45_Prepare(void) {
    if(sReady)return true;
    /* Native allocations are published only after all ROM reads validate.
     * ROM must remain the same until the game is reset (same as room loader). */
    if(!DsiGbaBus_Read(E45_ANIM_ROM,sAnimations,sizeof sAnimations))return false;
    for(unsigned i=0;i<E45_SPRITES;i++) {
        const E45Metadata* m=&e45Metadata[i];
        u32 fields[4];
        for(unsigned j=0;j<4;j++)
            if(!DsiRomRefs_ReadU32(EU_SPRITE_PTRS+m->sprite*16+j*4,&fields[j]))return false;
        if(fields[0]!=m->animations||fields[1]!=m->frames||
           fields[2]!=E45_GFX_ROM||fields[3])return false;
        for(unsigned j=0;j<=m->animCount;j++) {
            u32 address;
            if(!DsiRomRefs_ReadU32(m->animations+j*4,&address))return false;
            if(j==m->animCount) {if(address)return false;continue;}
            if(address<E45_ANIM_ROM||address>E45_ANIM_ROM+E45_ANIM_BYTES-4)return false;
            sOffsets[i][j]=address-E45_ANIM_ROM;
        }
        for(unsigned j=0;j<m->frameCount;j++) {
            u8 b[4];
            if(!DsiGbaBus_Read(m->frames+j*4,b,sizeof b))return false;
            SpriteFrame* f=&sFrames[i][j];
            f->numTiles=b[0];f->unk_1=b[1];f->firstTileIndex=b[2]|((u16)b[3]<<8);
            if(f->numTiles&&((u32)f->firstTileIndex+f->numTiles)*32>E45_GFX_BYTES)return false;
        }
    }
    sTiles=malloc(E45_GFX_BYTES);
    if(!sTiles)return false;
    if(!DsiGbaBus_Read(E45_GFX_ROM,sTiles,E45_GFX_BYTES)) {
        free(sTiles);sTiles=NULL;return false;
    }
    for(unsigned i=0;i<E45_SPRITES;i++) {
        sSprites[i].animations=sOffsets[i]; /* Read through Data(), never cast to GBA pointers. */
        sSprites[i].frames=sFrames[i];sSprites[i].ptr=sTiles;sSprites[i].pad=0;
    }
    sReady=true;return true;
}
const SpritePtr* DsiAnimationE45_Sprite(u16 sprite) {
    int slot=find_sprite(sprite);
    if(!sReady||slot<0)abort();
    return &sSprites[slot];
}
const u8* DsiAnimationE45_Data(u16 sprite,u32 animation) {
    if(!DsiAnimationE45_CanInitialize(sprite,animation))abort();
    return sAnimations+sOffsets[find_sprite(sprite)][animation];
}
SpriteFrame* DsiAnimationE45_Frame(u16 sprite,u8 frame) {
    int slot=find_sprite(sprite);
    if(!sReady||slot<0||frame>=e45Metadata[slot].frameCount)abort();
    return &sFrames[slot][frame];
}
int DsiAnimationE45_RequireBytes(const void* pointer,size_t count) {
    uintptr_t p=(uintptr_t)pointer,base=(uintptr_t)sAnimations;
    if(!sReady||p<base||p-base>sizeof sAnimations||count>sizeof sAnimations-(p-base))abort();
    return 1;
}
u8* DsiAnimationE45_LoopTarget(u8* next,u8 frames) {
    DsiAnimationE45_RequireBytes(next,1);
    size_t offset=(uintptr_t)next-(uintptr_t)sAnimations;
    if((size_t)frames*4>offset)abort();
    u8* target=sAnimations+offset-(size_t)frames*4;
    DsiAnimationE45_RequireBytes(target,4);
    return target;
}
