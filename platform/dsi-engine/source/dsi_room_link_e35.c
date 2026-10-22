/* E35 presentation bridge from ORIGINAL Link's native PlayerEntity.
 * Source ROM graphics routine TmcLink_Load/Draw already exists at
 * platform/dsi/source/tmc_link_sprite.c (EU GBA sprite assets/OAM).
 * This module never changes position, hitbox, action, input or collision. */
#include "dsi_room_link_e35.h"
#include "dsi_room_spawn_e31.h"
#include "tmc_link_sprite.h"
#include "entity.h"
#include "player.h"
#include "room.h"
#include <stdint.h>
#include <string.h>
static TmcLinkSprite sLinkRom;
static bool sLoaded;
static unsigned sErrors;
unsigned DsiRoomLinkE35_Errors(void) { return sErrors; }
bool DsiRoomLinkE35_Draw(const DsiRoomMetadata *room,TmcRomReader *rom,
                         uint16_t *out,size_t stride) {
    if (!room || !rom || !out || stride!=256u ||
        rom->region!=TMC_ROM_REGION_EUROPE ||
        room->area!=0x22u || room->room!=0x15u ||
        !DsiRoomSpawnE31_Ready() ||
        gPlayerEntity.base.kind!=PLAYER ||
        gPlayerEntity.base.next==NULL) return false;
    const Entity *player=&gPlayerEntity.base;
    /* Always use the game engine's world position. Never keep separate
     * demo coordinates for Link. All graphics remain external ROM data. */
    const int px=player->x.HALF.HI-room->map_x;
    const int py=player->y.HALF.HI-room->map_y;
    if(px<-32 || py<-32 || px>(int)room->width+32 ||
       py>(int)room->height+32) { ++sErrors; return false; }
    if(!sLoaded) {
        if(!TmcLink_Load(&sLinkRom,rom)) { ++sErrors; return false; }
        sLoaded=true;
    }
    /* Until the original animation state machine is live, the original
     * sprite's ROM frame 0 is a frame-accurate STATIC preview, not a
     * fabricated walk or wake-up sequence. */
    TmcLink_Draw(&sLinkRom,out,(int)stride,256,192,
                 px+8,py+16,0u,0u);
    return true;
}
