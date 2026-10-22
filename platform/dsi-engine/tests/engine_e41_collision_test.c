/* Synthetic ROM bus fixtures ONLY in this test. Original engine algorithms. */
#include "dsi_room_collision_e41.h"
#include "dsi_gba_bus.h"
#include "room.h"
#include "map.h"
#include "collision.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
MapLayer gMapBottom,gMapTop;
RoomControls gRoomControls;
static bool border_ready;
static unsigned fault;
bool DsiRoomCollisionE36_Ready(void){return border_ready;}
bool DsiGbaBus_Read(uint32_t address,void* dest,size_t size) {
    const u32 literals[]={0x08000258,0x0800025c,0x08000260,0x08000264};
    const u32 targets[]={0x080b7038,0x080b70cf,0x080b2ec8,0x080b35a8};
    for(unsigned i=0;i<4;i++)if(address==literals[i]&&size==4) {
        u32 p=targets[i]+(fault==1?4:0);u8* b=dest;
        for(unsigned k=0;k<4;k++)b[k]=p>>(k*8);
        return true;
    }
    if(fault==2)return false;
    u8* b=dest;
    if(address==targets[0]&&size==DSI_E41_SPECIAL_TILES) {
        for(unsigned i=0;i<size;i++)b[i]=(u8)(i+7);return true;
    }
    if(address==targets[1]&&size==DSI_E41_SPECIAL_TILES) {
        for(unsigned i=0;i<size;i++)b[i]=(u8)(i+13);return true;
    }
    if(address==targets[2]&&size==DSI_E41_TILE_TYPES) {
        for(unsigned i=0;i<size;i++)b[i]=GetMapTileTypeToActTile(i);
        if(fault==3)b[30]^=1;
        return true;
    }
    if(address==targets[3]&&size==DSI_E41_TILE_TYPES) {
        for(unsigned i=0;i<size;i++)b[i]=(u8)(i*17);return true;
    }
    return false;
}
static void unchanged(void) {
    for(unsigned i=0;i<4096;i++) {
        assert(gMapBottom.collisionData[i]==0xa5&&gMapTop.collisionData[i]==0xa5);
        assert(gMapBottom.actTiles[i]==0x5a&&gMapTop.actTiles[i]==0x5a);
    }
    assert(!DsiRoomCollisionE41_Ready());
}
int main(void) {
    DsiRoomMetadata r={0};r.area=0x22;r.room=0x15;r.width=240;r.height=160;
    gRoomControls.area=r.area;gRoomControls.room=r.room;
    gRoomControls.width=r.width;gRoomControls.height=r.height;
    for(unsigned l=0;l<2;l++) {
        MapLayer* m=l?&gMapTop:&gMapBottom;
        memset(m->collisionData,0xa5,sizeof m->collisionData);
        memset(m->actTiles,0x5a,sizeof m->actTiles);
        for(unsigned i=0;i<DSI_E41_TILE_TYPES;i++)m->tileTypes[i]=i;
        for(unsigned i=0;i<4096;i++)m->mapData[i]=i%DSI_E41_TILE_TYPES;
        for(unsigned i=0;i<DSI_E41_SPECIAL_TILES;i++)m->mapData[2000+i]=0x4000+i;
    }
    assert(!DsiRoomCollisionE41_Prepare(&r));unchanged();border_ready=true;
    assert(!DsiRoomCollisionE41_Prepare(NULL));unchanged();
    DsiRoomMetadata bad=r;bad.width=224;
    assert(!DsiRoomCollisionE41_Prepare(&bad));unchanged();
    u16 old=gMapTop.mapData[4095];
    gMapTop.mapData[4095]=TILESET_SIZE;
    assert(!DsiRoomCollisionE41_Prepare(&r));unchanged();
    gMapTop.mapData[4095]=0x4000+DSI_E41_SPECIAL_TILES;
    assert(!DsiRoomCollisionE41_Prepare(&r));unchanged();
    gMapTop.mapData[4095]=old;gMapTop.tileTypes[0]=DSI_E41_TILE_TYPES;
    assert(!DsiRoomCollisionE41_Prepare(&r));unchanged();gMapTop.tileTypes[0]=0;
    for(fault=1;fault<=3;fault++) {
        DsiRoomCollisionE41_Reset();
        assert(!DsiRoomCollisionE41_Prepare(&r));unchanged();
        assert(DsiRoomCollisionE41_Errors()==1);
    }
    fault=0;DsiRoomCollisionE41_Reset();assert(DsiRoomCollisionE41_Prepare(&r));
    assert(DsiRoomCollisionE41_Ready());
    for(unsigned l=0;l<2;l++) {
        MapLayer* m=l?&gMapTop:&gMapBottom;
        assert(!memcmp(m->mapDataOriginal,m->mapData,sizeof m->mapData));
        for(unsigned i=0;i<4096;i++) {
            unsigned t=m->mapData[i];
            u8 c=t<0x4000?(u8)(t*17):(u8)(t-0x4000+13);
            u8 a=t<0x4000?GetMapTileTypeToActTile(t):(u8)(t-0x4000+7);
            if(i%64==15||i%64==63||i/64==10||i/64==63)c=255;
            assert(m->collisionData[i]==c&&m->actTiles[i]==a);
        }
    }
    gMapBottom.collisionData[0]=42;
    assert(DsiRoomCollisionE41_Prepare(&r));assert(gMapBottom.collisionData[0]==42);
    border_ready=false;assert(!DsiRoomCollisionE41_Ready());
    puts("E41 original interior collision + act tiles: all types, special tiles, bounds, failure atomicity PASS");
}
