/* Transport, validation and call ordering only. Collision/act-tile rules
 * execute verbatim original playerUtils.c and scroll.c functions. */
#include "dsi_room_collision_e41.h"
#include "dsi_room_collision_e36.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "map.h"
#include "room.h"
#include "collision.h"
#include <string.h>
u8 gMapTileTypeToCollisionData[DSI_E41_TILE_TYPES];
u8 gMapSpecialTileToCollisionData[DSI_E41_SPECIAL_TILES];
u8 gMapSpecialTileToActTile[DSI_E41_SPECIAL_TILES];
static bool sReady;
static unsigned sErrors;
extern void sub_0807BBE4(void);
extern void CreateCollisionDataBorderAroundRoom(void);
extern void FillActTileForLayer(MapLayer*);
void DsiRoomCollisionE41_Reset(void) {sReady=false;sErrors=0;}
unsigned DsiRoomCollisionE41_Errors(void) {return sErrors;}
bool DsiRoomCollisionE41_Ready(void) {
    return sReady && !sErrors && DsiRoomCollisionE36_Ready();
}
static bool valid_layer(const MapLayer* m) {
    for(unsigned i=0;i<4096u;i++) {
        unsigned tile=m->mapData[i];
        if(tile<0x4000u) {
            if(tile>=TILESET_SIZE || m->tileTypes[tile]>=DSI_E41_TILE_TYPES)return false;
        } else if(tile-0x4000u>=DSI_E41_SPECIAL_TILES)return false;
    }
    return true;
}
static bool table(u32 literal,u32 expected,void* dst,size_t bytes) {
    u32 ptr;
    return DsiRomRefs_ReadU32(literal,&ptr) && ptr==expected &&
        DsiRomRefs_IsRomSpan(ptr,bytes) && DsiGbaBus_Read(ptr,dst,bytes);
}
bool DsiRoomCollisionE41_Prepare(const DsiRoomMetadata* room) {
    if(!room || sErrors || !DsiRoomCollisionE36_Ready() ||
       room->area!=0x22 || room->room!=0x15 ||
       room->area!=gRoomControls.area || room->room!=gRoomControls.room ||
       room->width!=gRoomControls.width || room->height!=gRoomControls.height ||
       room->map_x!=gRoomControls.origin_x || room->map_y!=gRoomControls.origin_y)
        return false;
    if(sReady)return true; /* do not overwrite live tile changes */
    if(!valid_layer(&gMapBottom)||!valid_layer(&gMapTop))return false;
    /* EU SetTile literal pool: asm/src/veneer.s + EU header delta 0x48.
     * Follow AND verify identities instead of treating GBA values as pointers.
     * 0x97 special entries: special-act to special-collision table distance.
     * Only the 1396 types supported by the native original act table are allowed. */
    if(!table(0x08000264u,0x080b35a8u,gMapTileTypeToCollisionData,sizeof gMapTileTypeToCollisionData)||
       !table(0x0800025cu,0x080b70cfu,gMapSpecialTileToCollisionData,sizeof gMapSpecialTileToCollisionData)||
       !table(0x08000258u,0x080b7038u,gMapSpecialTileToActTile,sizeof gMapSpecialTileToActTile)) {
        ++sErrors;return false;
    }
    /* Cross-check the existing native regional act table against this ROM. */
    u8 acts[DSI_E41_TILE_TYPES];
    if(!table(0x08000260u,0x080b2ec8u,acts,sizeof acts)) {++sErrors;return false;}
    for(unsigned i=0;i<sizeof acts;i++)
        if(acts[i]!=GetMapTileTypeToActTile(i)) {++sErrors;return false;}
    memcpy(gMapBottom.mapDataOriginal,gMapBottom.mapData,sizeof gMapBottom.mapData);
    memcpy(gMapTop.mapDataOriginal,gMapTop.mapData,sizeof gMapTop.mapData);
    sub_0807BBE4();
    CreateCollisionDataBorderAroundRoom();
    FillActTileForLayer(&gMapBottom);
    FillActTileForLayer(&gMapTop);
    sReady=true;
    return true;
}
