/* Transport only: original vram.c chooses slots, dirty states and addresses. */
#include "dsi_sprite_upload_e46.h"
#include "dsi_animation_e45.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "dsi_title_assets.h"
#include "structures.h"
#include <stdlib.h>
static u8 sRomTransfer[0x8000];
static bool obj_range(uintptr_t address,size_t bytes) {
    return address>=0x06010000u&&address<=0x06018000u&&bytes<=0x06018000u-address;
}
void DsiSpriteUploadE46_ValidateSlot(unsigned index) {
    if(index>=MAX_GFX_SLOTS)abort();
    const GfxSlot* slot=&gGFXSlots.slots[index];
    /* Original upload clears each following 16-tile block's paletteIndex. */
    if(slot->paletteIndex!=0xffff&&
       (slot->paletteIndex+15u)/16u>MAX_GFX_SLOTS-index)abort();
}
void DsiSpriteUploadE46_Copy(uintptr_t source,uintptr_t destination,size_t bytes) {
    if(!obj_range(destination,bytes)||(destination&3)||(source&3)||(bytes&3))abort();
    const void* data=(const void*)source;
    if(!DsiAnimationE45_IsTileSpan(data,bytes)) {
        if(source>UINT32_MAX||!DsiRomRefs_IsRomSpan((u32)source,bytes)||
           bytes>sizeof sRomTransfer||!DsiGbaBus_Read((u32)source,sRomTransfer,bytes))abort();
        data=sRomTransfer;
    }
    if(!DsiGbaBus_Write((u32)destination,data,bytes))abort();
}
void DsiSpriteUploadE46_Lz(const void* source,void* destination) {
    uintptr_t src=(uintptr_t)source,dst=(uintptr_t)destination;
    u8 header[4];u32 written;
    if(src>UINT32_MAX||!DsiRomRefs_IsRomSpan((u32)src,4)||
       !DsiGbaBus_Read((u32)src,header,4)||header[0]!=0x10)abort();
    u32 bytes=header[1]|((u32)header[2]<<8)|((u32)header[3]<<16);
    if(!bytes||!obj_range(dst,bytes)||(dst&1)||
       !DsiTitleAssets_UnpackLz77((u32)src,(u32)dst,&written)||written!=bytes)abort();
}
