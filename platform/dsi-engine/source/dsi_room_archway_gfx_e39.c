/* E39: render the authentic EU Archway sprite from its actual
 * compiled EU ObjectDefinition, fixed GFX ROM record, palette and
 * original frame OAM commands. This is a presentation bridge, not an
 * object/Link gameplay implementation. Never ship ROM assets in NDS. */
#include "dsi_room_archway_gfx_e39.h"
#include "dsi_room_archway_e39.h"
#include "dsi_title_assets.h"
#include "dsi_title_sprites.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "definitions.h"
#include "entity.h"
#include "object.h"
#include "room.h"
#include "screen.h"
#include <string.h>
#define EU_FIXED_GFX_TABLE 0x08132180u
#define EU_GFX_AND_PALETTES 0x085A23D0u
#define EU_FIXED_COUNT 525u
#define EU_OBJ_VRAM 0x06012800u
#define EU_BASE_TILE 0x140u
#define PALETTE_BANK 6u
#define ROOM_FRAME_WIDTH 256u
#define ROOM_FRAME_HEIGHT 192u
extern const ObjectDefinition gObjectDefinition_4F[];
extern uint8_t gVram[0x18000];
extern uint16_t gObjPltt[256];
static uint16_t stage[ROOM_FRAME_WIDTH*ROOM_FRAME_HEIGHT];
static unsigned sErrors;
static bool fail(void) { ++sErrors; return false; }
unsigned DsiRoomArchwayGfxE39_Errors(void) { return sErrors; }
bool DsiRoomArchwayGfxE39_Draw(const DsiRoomMetadata* room,
                            uint16_t* out, size_t stride) {
    if (!room || !out || stride!=ROOM_FRAME_WIDTH ||
        room->area!=0x22 || room->room!=0x15 ||
        !DsiRoomArchwayE39_Ready()) return false;
    Entity* arch=DeepFindEntityByID(OBJECT, ARCHWAY);
    if (!arch || arch->type!=1 || arch->action!=1 ||
        arch->frameIndex!=arch->type2 || !arch->spriteSettings.draw) return fail();
    /* Original EU type-1 Archway definition: gfx99, palette10, sprite407.
     * Refuse to render if upstream source definitions change unexpectedly. */
    const ObjectDefinition* def=&gObjectDefinition_4F[arch->type];
    if (def->bitfield.type!=1u || def->bitfield.gfx_type!=0u ||
        def->bitfield.gfx!=99u || def->data.sprite.paletteIndex!=10u ||
        def->data.sprite.spriteIndex!=407u ||
        def->bitfield.gfx>=EU_FIXED_COUNT) return fail();
    uint32_t record;
    if (!DsiRomRefs_ReadU32(EU_FIXED_GFX_TABLE+def->bitfield.gfx*4u,&record))
        return fail();
    uint32_t slots=(record>>24)&0x7fu;
    if (!slots || slots>32u) return fail();
    uint32_t bytes=slots*512u;
    /* Source-derived rule: original src/vram.c::sub_080ADDD8 masks the
     * packed GFX record by 0xFFFFFC before adding gGlobalGfxAndPalettes.
     * The 2 low bits are loader flags, NOT file offset bytes. */
    uint32_t src=EU_GFX_AND_PALETTES+(record&0x00fffffcu);
    if (!DsiRomRefs_IsRomSpan(src,4) ||
        !DsiGbaBus_IsWritable(EU_OBJ_VRAM,bytes)) return fail();
    if(record&1u) {
        uint8_t hdr[4];
        uint32_t produced;
        if(!DsiGbaBus_Read(src,hdr,sizeof hdr) || hdr[0]!=0x10u)
            return fail();
        uint32_t expected=(uint32_t)hdr[1]|((uint32_t)hdr[2]<<8)|((uint32_t)hdr[3]<<16);
        if(!expected || expected>bytes ||
           !DsiTitleAssets_UnpackLz77(src,EU_OBJ_VRAM,&produced) ||
           produced!=expected) return fail();
    } else if (!DsiRomRefs_IsRomSpan(src,bytes) ||
               !DsiGbaBus_Copy(EU_OBJ_VRAM,src,bytes)) return fail();
    /* The original GBA palette group uses 16-color records of 32 bytes.
     * This is only the single object preview; do not bind a native
     * palette allocator until the full original renderer is linked. */
    const uint32_t palSrc=EU_GFX_AND_PALETTES+
                          def->data.sprite.paletteIndex*32u;
    const uint32_t palDst=0x05000200u+PALETTE_BANK*32u;
    if(!DsiRomRefs_IsRomSpan(palSrc,32u) ||
       !DsiGbaBus_Copy(palDst,palSrc,32u)) return fail();
    int x=arch->x.HALF.HI-room->map_x;
    int y=arch->y.HALF.HI-room->map_y;
    if (x < -64 || y < -64 || x > 320 || y > 224) return fail();
    OAMCommand cmd={0};
    cmd.x=(int16_t)x; cmd.y=(int16_t)y;
    cmd._8=EU_BASE_TILE;
    DsiTitleSprites_Clear();
    if(!DsiTitleSprites_DrawDirect(def->data.sprite.spriteIndex,
                                  arch->frameIndex,&cmd) ||
       !DsiTitleSprites_UsePaletteBank(PALETTE_BANK)) return fail();
    memcpy(stage,out,sizeof stage);
    unsigned pixels=0;
    if(!DsiTitleSprites_Composite(stage,ROOM_FRAME_WIDTH,gVram,sizeof(gVram),
                                  gObjPltt,256u,gScreen.lcd.displayControl,
                                  &pixels) || !pixels) return fail();
    memcpy(out,stage,sizeof stage);
    return true;
}
