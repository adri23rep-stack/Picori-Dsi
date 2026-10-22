/* E35: read-only validation of original collision map data, not
 * alternative Zelda physics or a synthetic walk implementation. */
#include "dsi_collision_audit_e35.h"
#include "dsi_room_spawn_e31.h"
#include "player.h"
#include "entity.h"
#include "room.h"
#include "map.h"
#include <stdint.h>
extern u32 GetCollisionDataAtTilePos(u32,u32);
extern u32 GetCollisionDataAtWorldCoords(u32,u32,u32);
extern u32 GetCollisionDataAtEntity(Entity*);
extern u32 GetCollisionDataRelativeTo(Entity*,s32,s32);
static unsigned sNonzero,sErrors;
unsigned DsiCollisionAuditE35_NonzeroTiles(void){return sNonzero;}
unsigned DsiCollisionAuditE35_Errors(void){return sErrors;}
bool DsiCollisionAuditE35_Run(unsigned width,unsigned height){
    sNonzero=0;
    if(!DsiRoomSpawnE31_Ready() || !width || !height ||
       width>1024u || height>1024u || (width&15u) || (height&15u) ||
       (gPlayerEntity.base.collisionLayer!=1 &&
        gPlayerEntity.base.collisionLayer!=2)){
        ++sErrors;return false;
    }
    Entity* link=&gPlayerEntity.base;
    int32_t x=(int32_t)link->x.HALF.HI-(int32_t)gRoomControls.origin_x;
    int32_t y=(int32_t)link->y.HALF.HI-(int32_t)gRoomControls.origin_y;
    if(x<0 || y<0 || x>=(int32_t)width || y>=(int32_t)height){
        ++sErrors;return false;
    }
    for(unsigned iy=0;iy<height/16u;iy++)
       for(unsigned ix=0;ix<width/16u;ix++){
           unsigned pos=ix+iy*64u;
           unsigned c1=GetCollisionDataAtTilePos(pos,1);
           unsigned c2=GetCollisionDataAtTilePos(pos,2);
           if(c1!=gMapBottom.collisionData[pos] ||
              c2!=gMapTop.collisionData[pos]){
              ++sErrors;return false;
           }
           sNonzero+=(c1!=0u)+(c2!=0u);
       }
    if(GetCollisionDataAtEntity(link)!=
       GetCollisionDataAtWorldCoords((unsigned)link->x.HALF.HI,
                                    (unsigned)link->y.HALF.HI,
                                    link->collisionLayer) ||
       GetCollisionDataRelativeTo(link,0,0)!=GetCollisionDataAtEntity(link)){
        ++sErrors;return false;
    }
    /* Empty/uninitialized collision maps are NOT walkable level data.
     * Original movement and collision resolution remain disabled. */
    return sNonzero>0;
}
