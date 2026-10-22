#include "dsi_room_spawn_e31.h"
#include "dsi_room_collision_e36.h"
#include "dsi_room_collision_e41.h"
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include "dsi_room_tiles.h"
#include "dsi_title_assets.h"
#include "dsi_gba_bus.h"
#include "area.h"
#include "player.h"
#include "room.h"
#include "game.h"
#include "map.h"
#include "common.h"
#include "port_region_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern void ClearAllDeletedEntities(void);
Input gInput;
PlayerState gPlayerState;
int main(int argc,char**argv){
    gInput.heldKeys=A_BUTTON|DPAD_RIGHT;
    gPlayerState.controlMode=CONTROL_ENABLED;
    DsiRoomSpawnE31_PollInput();assert(!gPlayerState.playerInput.heldInput);
    assert(argc==2);TmcRomReader reader;memset(&reader,0,sizeof reader);
    assert(TmcRom_Open(&reader,argv[1]));assert(reader.region==TMC_ROM_REGION_EUROPE);
    DsiGbaBus_Init();DsiGbaBus_AttachRom(&reader);DsiTitleAssets_Enable(true,2);
    DsiRoomMetadata room;DsiRoomMapStats maps;DsiRoomTileStats tiles;
    assert(DsiRoomProbe_ReadEU(0x22,0x15,&room));
    assert(DsiRoomMaps_LoadEU(&room,&maps));assert(DsiRoomTiles_LoadEU(&room,&tiles));
    assert(DsiEntityLists_PrepareEU(&room));assert(DsiRoomPropertiesE30_PrepareEU(&room));assert(DsiRoomPropertyDataE30_Bind());
    EraseAllEntities();zMallocInit();gRoomControls.area=room.area;gRoomControls.room=room.room;
    gRoomControls.origin_x=room.map_x;gRoomControls.origin_y=room.map_y;
    RoomResInfo* info=&gArea.roomResInfos[room.room];gArea.pCurrentRoomInfo=info;
    info->map_x=room.map_x;info->map_y=room.map_y;info->pixel_width=room.width;info->pixel_height=room.height;
    static MapDataDefinition fixture;info->tileSet=&fixture;info->map=&fixture;info->tiles=&fixture; /* test fixture: non-NULL resource identity */
    gPlayerEntity.base.kind=PLAYER;gPlayerEntity.base.health=24;AppendEntityToList(&gPlayerEntity.base,1);
    assert(DsiRoomPropertyNativeE30_Attach(&room));
    const EntityData* bank0=DsiRoomPropertyDataE30_EntityBank(0);
    assert(DsiRoomPropertyDataE30_AllowNativeCopy(bank0,sizeof *bank0));
    assert(!DsiRoomPropertyDataE30_AllowNativeCopy((const char*)bank0+1,sizeof *bank0));
    assert(!DsiRoomPropertyDataE30_AllowNativeCopy(bank0,2*sizeof *bank0));
    assert(!DsiRoomPropertyDataE30_AllowNativeCopy(bank0+room.entity_counts[0],sizeof *bank0));
    assert(!DsiRoomPropertyDataE30_AllowNativeCopy((void*)(uintptr_t)room.entity_list_addresses[0],sizeof *bank0));
    assert(DsiRoomSpawnE31_Load(&room));assert(DsiRoomSpawnE31_Ready());
    assert(DsiRoomCollisionE36_Prepare(&room));
    assert(DsiRoomCollisionE41_Prepare(&room));
    assert(DsiRoomCollisionE41_Ready());
    unsigned walls=0,empty=0;
    for(unsigned l=0;l<2;l++) {
        MapLayer* m=l?&gMapTop:&gMapBottom;
        assert(!memcmp(m->mapData,m->mapDataOriginal,sizeof m->mapData));
        for(unsigned i=0;i<4096;i++) {
            unsigned tile=m->mapData[i];u8 collision,act;
            unsigned type=tile<0x4000?m->tileTypes[tile]:tile-0x4000;
            assert(DsiGbaBus_Read((tile<0x4000?0x080b35a8:0x080b70cf)+type,&collision,1));
            assert(DsiGbaBus_Read((tile<0x4000?0x080b2ec8:0x080b7038)+type,&act,1));
            unsigned col=i%64,row=i/64;
            if(col==room.width/16 || col==63 || row==room.height/16 || row==63)collision=255;
            assert(m->collisionData[i]==collision);assert(m->actTiles[i]==act);
            if(col<room.width/16&&row<room.height/16) {if(collision)walls++;else empty++;}
        }
    }
    assert(walls&&empty);printf("E41 ROM: %u blocking, %u empty interior cells; both full layers exact\n",walls,empty);
    s32 x=gPlayerEntity.base.x.WORD,y=gPlayerEntity.base.y.WORD;
    DsiRoomSpawnE31_PollInput();
    assert(gPlayerState.playerInput.heldInput&INPUT_RIGHT);
    assert(gPlayerState.playerInput.newInput&INPUT_INTERACT);
    DsiRoomSpawnE31_PollInput();assert(!gPlayerState.playerInput.newInput);
    gPlayerState.controlMode=CONTROL_DISABLED;DsiRoomSpawnE31_PollInput();
    assert(!gPlayerState.playerInput.heldInput);
    assert(gPlayerState.controlMode==CONTROL_DISABLED);
    assert(gPlayerEntity.base.x.WORD==x&&gPlayerEntity.base.y.WORD==y);
    assert(gEntCount==2&&gManagerCount==3);
    Entity* bed=DeepFindEntityByID(OBJECT,0x4f);assert(bed&&bed->type==1);
    assert(bed->x.HALF.HI==room.map_x+0x58 && bed->y.HALF.HI==room.map_y+8);
    assert(bed->collisionLayer>=1&&bed->collisionLayer<=3);
    /* Persistence, not a spawn-and-rollback diagnostic. Read-only check again. */
    assert(!DsiRoomSpawnE31_Load(&room));assert(DeepFindEntityByID(OBJECT,0x4f)==bed);
    assert(gEntCount==2&&gManagerCount==3&&gPlayerEntity.base.health==24);
    assert(DsiGbaBus_InvalidReads()==0&&DsiGbaBus_InvalidWrites()==0);
    printf("E31 EU ROM: 3 managers + original bed persist; layer=%u; Link preserved; no updates dispatched\n",bed->collisionLayer);
    EraseAllEntities();assert(gEntCount==0&&gManagerCount==0);
    TmcRom_Close(&reader);
}
