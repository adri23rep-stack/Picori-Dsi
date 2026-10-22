/* Stable native ROM data for the existing translated original collision code.
 * No invented wall tests, velocities or player actions. */
#include "dsi_collision_probe_e42.h"
#include "dsi_room_collision_e41.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "port_config.h"
#include "map.h"
#include "hitbox.h"
#include <stdlib.h>
u8 gDsiE42CollisionParams[7][96];
static u16 sShapes[40][16];
static bool sReady;
extern void sub_080085CC(Entity*);
void DsiCollisionE42_Reset(void) {sReady=false;}
bool DsiCollisionE42_Ready(void) {return sReady&&DsiRoomCollisionE41_Ready();}
static bool valid_maps(void) {
    for(unsigned i=0;i<4096;i++) {
        unsigned a=gMapBottom.collisionData[i],b=gMapTop.collisionData[i];
        if((a>=0x70&&a!=255)||(b>=0x70&&b!=255))return false;
    }
    return true;
}
bool DsiCollisionE42_Prepare(void) {
    if(!DsiRoomCollisionE41_Ready()||!valid_maps())return false;
    if(sReady)return true;
    /* assets/assets.json: seven consecutive 96-byte parameter banks start
     * at USA 0x82dc, regional EU adjustment +0x98 -> 0x8374. */
    if(!DsiGbaBus_Read(0x08008374,gDsiE42CollisionParams,sizeof gDsiE42CollisionParams))return false;
    for(unsigned b=0;b<7;b++)for(unsigned i=0;i<96;i++)
        if(gDsiE42CollisionParams[b][i]>=40)return false;
    for(unsigned i=0;i<40;i++) {
        u32 p;u8 bytes[32];
        if(!DsiRomRefs_ReadU32(0x08000000u+PORT_COLLISION_SHAPE_PTRS_EU+4*i,&p)||
           (p&1)||!DsiRomRefs_IsRomSpan(p,sizeof bytes)||!DsiGbaBus_Read(p,bytes,sizeof bytes))return false;
        for(unsigned row=0;row<16;row++)sShapes[i][row]=bytes[2*row]|((u16)bytes[2*row+1]<<8);
    }
    sReady=true;return true;
}
const u16* DsiCollisionE42_Shape(u32 index) {
    /* Original lookup's NULL fallback is never used as fake passability. */
    if(!DsiCollisionE42_Ready()||index>=40)abort();
    return sShapes[index];
}
void* DsiCollisionE42_Hitbox(uintptr_t pointer) {
    if(pointer!=(uintptr_t)&gPlayerHitbox)abort();
    return (void*)&gPlayerHitbox;
}
bool DsiCollisionE42_Point(const Entity* e,u32 x,u32 y,u32* blocked) {
    if(!e||!blocked||!DsiCollisionE42_Ready()||e->collisionLayer>3)return false;
    MapLayer* m=e->collisionLayer==2?&gMapTop:&gMapBottom;
    unsigned c=m->collisionData[((x&0x3f0)>>4)+((y&0x3f0)<<2)];
    if(c>=0x70&&c!=255)return false;
    *blocked=DsiCollisionE42_OriginalPoint((Entity*)e,x,y);return true;
}
bool DsiCollisionE42_Probe(Entity* e) {
    /* Actual PlayerInit has to install its hitbox; this bridge never does. */
    if(!e||!DsiCollisionE42_Ready()||e->collisionLayer>3||
       e->hitbox!=&gPlayerHitbox||!valid_maps())return false;
    sub_080085CC(e);return true;
}
