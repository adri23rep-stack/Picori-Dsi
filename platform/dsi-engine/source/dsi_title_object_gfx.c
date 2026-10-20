/* Original source data: gObjectDefinition_BD, compiled unmodified from
 * src/objectDefinitions.c. Original EU ROM fixed graphics/palette tables are
 * referenced by their retail GBA addresses; no textures are bundled. */
#include "dsi_title_object_gfx.h"
#include "dsi_title_assets.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "definitions.h"
#include <string.h>

/* EU offsets from port/port_rom.c kRomOffsets_EU. */
#define EU_FIXED_GFX_TABLE 0x08132180u
#define EU_OBJ_PALETTE_TABLE 0x081329B4u
#define EU_GLOBAL_DATA 0x085A23D0u
#define EU_FIXED_COUNT 525u
#define EU_OBJ_PALETTE_COUNT 360u
#define OBJ_VRAM_TILE_BASE 0x06012800u
/* Original slot allocation begins at 4 (0-3 reserved). Gfx 511 has 11
 * slots, gfx 512 5, gfx 513 and 514 one each: [4,15,20,21]. */
#define SLOT_CAPACITY 64u
#define PALETTE_CAPACITY 16u
extern const ObjectDefinition gObjectDefinition_BD[5];

static struct { uint16_t id; uint16_t first, slots; } graphics[4];
static struct { uint16_t id; uint8_t slot; } palettes[3];
static unsigned ngraphics, npalettes, nextslot;
DsiTitleObjectGfxStats gDsiTitleObjectGfxStats;

static bool bad(void) { ++gDsiTitleObjectGfxStats.errors; return false; }
static bool readword(uint32_t addr, uint32_t *val) { return DsiRomRefs_ReadU32(addr,val); }
static bool romspan(uint32_t address, uint32_t size) {
    return address >= 0x08000000u && address < 0x09000000u &&
        size <= 0x09000000u - address;
}
void DsiTitleObjectGfx_Reset(void) {
    ngraphics=npalettes=0;
    nextslot=4u;
    memset(graphics,0,sizeof(graphics));
    memset(palettes,0,sizeof(palettes));
    memset(&gDsiTitleObjectGfxStats,0,sizeof(gDsiTitleObjectGfxStats));
}
static bool load_graphic(unsigned id, unsigned *tile_base) {
    if(!id || id>=EU_FIXED_COUNT || !tile_base) return bad();
    for(unsigned i=0;i<ngraphics;i++) if(graphics[i].id==id) {
        *tile_base=graphics[i].first * 16u + 0x140u;
        ++gDsiTitleObjectGfxStats.gfx_reused;
        return true;
    }
    if(ngraphics>=4) return bad();
    uint32_t record;
    if(!readword(EU_FIXED_GFX_TABLE+id*4u,&record)) return bad();
    unsigned slots=(record>>24)&0x7fu;
    if(!slots || slots>SLOT_CAPACITY-nextslot) return bad();
    uint32_t length=slots*512u;
    uint32_t src=EU_GLOBAL_DATA+(record&0x00ffffffu);
    uint32_t dst=OBJ_VRAM_TILE_BASE+nextslot*512u;
    if(!romspan(src,4) || !DsiGbaBus_IsWritable(dst,length)) return bad();
    if(record&1u) { /* the original gfx slot uses LZ77 for odd offsets */
        uint32_t produced=0;
        if(!DsiTitleAssets_UnpackLz77(src,dst,&produced) || produced>length) return bad();
        length=produced;
    } else if(!romspan(src,length) || !DsiGbaBus_Copy(dst,src,length)) return bad();
    *tile_base=nextslot*16u+0x140u;
    graphics[ngraphics].id=(uint16_t)id;
    graphics[ngraphics].first=(uint16_t)nextslot;
    graphics[ngraphics].slots=(uint16_t)slots;
    ++ngraphics;
    nextslot+=slots;
    ++gDsiTitleObjectGfxStats.gfx_loaded;
    gDsiTitleObjectGfxStats.gfx_bytes+=length;
    return true;
}
static bool load_palette(unsigned id,unsigned *out_slot) {
    if(!out_slot || id<0x16u || id-0x16u>=EU_OBJ_PALETTE_COUNT) return bad();
    for(unsigned i=0;i<npalettes;i++) if(palettes[i].id==id) {
        *out_slot=palettes[i].slot;
        ++gDsiTitleObjectGfxStats.pal_reused;
        return true;
    }
    if(npalettes>=3u) return bad();
    uint32_t rec;
    if(!readword(EU_OBJ_PALETTE_TABLE+(id-0x16u)*4u,&rec)) return bad();
    unsigned count=(rec>>24)&15u;
    /* E12 supports exactly one 16-color bank per title-object palette. */
    if(count!=1u || 6u+npalettes>=PALETTE_CAPACITY) return bad();
    uint32_t src=EU_GLOBAL_DATA+(rec&0x00ffffffu), bytes=count*32u;
    uint32_t dst=0x05000200u+(6u+npalettes)*32u;
    if(!romspan(src,bytes) || !DsiGbaBus_Copy(dst,src,bytes)) return bad();
    *out_slot=6u+npalettes;
    palettes[npalettes].id=(uint16_t)id;
    palettes[npalettes].slot=(uint8_t)*out_slot;
    ++npalettes;
    ++gDsiTitleObjectGfxStats.pal_loaded;
    gDsiTitleObjectGfxStats.pal_bytes+=bytes;
    return true;
}
bool DsiTitleObjectGfx_InitEntity(Entity *ent) {
    if(!ent || ent->type>=5u) return bad();
    const ObjectDefinition *def=&gObjectDefinition_BD[ent->type];
    if(def->bitfield.type!=1u || def->bitfield.gfx_type!=0u) return bad();
    unsigned gfxoffset = 0, palette = 0;
    if(!load_graphic(def->bitfield.gfx,&gfxoffset) ||
       !load_palette(def->data.sprite.paletteIndex,&palette)) return false;
    ent->spriteVramOffset=(uint16_t)gfxoffset;
    ent->spriteIndex=def->data.sprite.spriteIndex;
    ent->palette.raw=(uint8_t)palette;
    ent->spriteSettings.draw=def->data.sprite.draw;
    return true;
}
