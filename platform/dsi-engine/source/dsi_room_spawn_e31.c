/* Transport of original room data into native Zelda structures.
 * No entity behavior, movement, scripts or collision algorithms live here.
 * E31 only loads the verified initial room banks. UPDATE remains disabled. */
#include "dsi_room_spawn_e31.h"
#include "dsi_room_property_native_e30.h"
#include "dsi_room_property_data_e30.h"
#include "dsi_gba_bus.h"
#include "room.h"
#include "area.h"
#include "map.h"
#include "player.h"
#include "script.h"
#include "collision.h"
#include "port_region_data.h"
#include <stdlib.h>
#include <string.h>
MapLayer gMapBottom, gMapTop;
static bool sReady;
static unsigned sErrors;
void InitScriptData(void);
void sub_0807C8B0(u16*,u32,u32);
void DsiE31_UnsupportedTile(void) { ++sErrors; abort(); }
unsigned DsiRoomSpawnE31_Errors(void) { return sErrors; }
bool DsiRoomSpawnE31_Ready(void) { return sReady && !sErrors; }
/* This resolver accepts only complete, validated native bank records.
 * In particular 0x08xxxxxx and arbitrary native pointers are never accepted. */
const void* Port_ResolveRegionData(const void* data) {
    for(unsigned bank=0;bank<2;bank++) {
        const EntityData* records=DsiRoomPropertyDataE30_EntityBank(bank);
        unsigned count=DsiRoomPropertyDataE30_Count(bank);
        for(unsigned i=0;records && i<=count;i++)
            if(data==&records[i])return data;
    }
    ++sErrors; return NULL;
}
/* The initial room's two banks contain no scripted records. Unknown scripts
 * are not passed to StartCutscene with a NULL or raw GBA pointer. Stop loudly.
 * Real StartCutscene/context allocation/free are linked and tested separately. */
void* DsiE31_ResolveEntityScript(const void* data,u32 address) {
    (void)data;(void)address; ++sErrors; abort();
}
static bool map_layer(MapLayer* map,uint32_t base,const DsiRoomMetadata* room) {
    /* Original 32-bit MapLayer field offsets, read through the bounded bus.
     * Copy the fields separately: host pointer sizes must not affect offsets. */
    memset(map,0,sizeof *map);
    if(!DsiGbaBus_Read(base+4,map->mapData,sizeof map->mapData) ||
       !DsiGbaBus_Read(base+0x5004,map->tileTypes,sizeof map->tileTypes) ||
       !DsiGbaBus_Read(base+0x7004,map->subTiles,sizeof map->subTiles))return false;
    unsigned count=(room->width/16u)*(room->height/16u);
    for(unsigned i=0;i<count;i++) {
        unsigned index=map->mapData[i];
        if(index>=TILESET_SIZE || map->tileTypes[index]>=1396u)return false;
    }
    /* Original expansion from packed rows to the engine's 64-tile stride. */
    sub_0807C8B0(map->mapData,room->width/16u,room->height/16u);
    return true;
}
static bool lists_valid(void) {
    unsigned entities=0,managers=0;
    for(unsigned l=0;l<9;l++) {
        Entity* sentinel=(Entity*)&gEntityLists[l];Entity* previous=sentinel;
        unsigned steps=0;
        for(Entity* e=sentinel->next;e!=sentinel;e=e->next) {
            extern int Port_IsValidEntityAddr(const void*);
            if(!e || !Port_IsValidEntityAddr(e) || ++steps>MAX_ENTITIES+32 || e->prev!=previous)return false;
            if(e->kind==MANAGER)managers++;else entities++;
            previous=e;
        }
        if(sentinel->prev!=previous)return false;
    }
    return entities==gEntCount && managers==gManagerCount;
}
bool DsiRoomSpawnE31_Load(const DsiRoomMetadata* room) {
    if(sReady || sErrors || !room || room->area!=0x22 || room->room!=0x15 ||
       !DsiRoomPropertyNativeE30_Ready() || gEntCount!=1 || gManagerCount ||
       !room->width || !room->height || room->width>1024 || room->height>1024 ||
       (room->width&15) || (room->height&15))return false;
    unsigned ents=0,managers=0;
    /* Validate BOTH complete banks before the first real allocation. */
    for(unsigned bank=0;bank<2;bank++) {
        const EntityData* r=DsiRoomPropertyDataE30_EntityBank(bank);
        unsigned count=DsiRoomPropertyDataE30_Count(bank);
        if(!r || count>=DSI_ENTITY_RECORD_MAX || r[count].kind!=0xff)return false;
        for(unsigned i=0;i<count;i++) {
            unsigned kind=r[i].kind&15,mode=r[i].flags&0xf0,list=r[i].flags&15;
            if((kind!=MANAGER && kind!=OBJECT) || (list>8 && list!=15) ||
               mode!=0 || (r[i].kind&0xc0))return false;
            if(kind==MANAGER)managers++;else ents++;
        }
    }
    if(ents+1>=MAX_ENTITIES-1 || managers>32 || !lists_valid())return false;
    if(!map_layer(&gMapBottom,0x02025eb0u,room) ||
       !map_layer(&gMapTop,0x0200b650u,room)) { ++sErrors;return false; }
    /* StartCutscene initialization exists, but scripts are not interpreted. */
    InitScriptData();
    LoadRoomEntityList(DsiRoomPropertyDataE30_EntityBank(0));
    LoadRoomEntityList(DsiRoomPropertyDataE30_EntityBank(1));
    if(gEntCount!=1+ents || gManagerCount!=managers || !lists_valid()) { ++sErrors;return false; }
    sReady=true;
    return true;
}
