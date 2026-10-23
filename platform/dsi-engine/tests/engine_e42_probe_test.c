#include "dsi_collision_probe_e42.h"
#include "dsi_gba_bus.h"
#include "player.h"
#include "room.h"
#include "map.h"
#include "hitbox.h"
#include "physics.h"
#include "port_config.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
MapLayer gMapBottom,gMapTop;
RoomControls gRoomControls;
PlayerState gPlayerState;
PlayerEntity gPlayerEntity;
static bool ready;
static unsigned fault;
static void movement_tests(void) {
    /* FixedMul must truncate toward zero before the position-scale multiply.
       Cover every signed input, negative fractions and extreme speeds. */
    const s16 speeds[]={-32768,-513,-257,-1,0,1,255,256,257,513,32767};
    for(int a=-32768;a<=32767;a++)for(unsigned i=0;i<sizeof speeds/sizeof *speeds;i++)
        assert(FixedMul(a,speeds[i])==(s16)((a*(int)speeds[i])/256));
    /* Exact cardinal wall-slide cases from the Thumb branch conditions.
       Each row checks forward, first escape, second escape and enclosure. */
    const unsigned dirs[]={0,8,16,24};
    const unsigned block[]={0x000e,0xe000,0x00e0,0x0e00};
    const unsigned first[]={8,0,8,0}, second[]={24,16,24,16};
    const unsigned firstMask[]={0xe004,0x200e,0xe040,0x020e};
    const unsigned secondMask[]={0x0e02,0x40e0,0x0e20,0x04e0};
    const int dx[]={0,1,0,-1},dy[]={-1,0,1,0};
    for(unsigned q=0;q<4;q++)for(unsigned collision=0;collision<=65535;collision++) {
        Entity e={0};e.collisions=collision;e.x.WORD=0x12345678;e.y.WORD=0x23456789;
        unsigned d=dirs[q],speed=0x280;
        if(collision&block[q]) {
            if(!(collision&firstMask[q]))d=first[q];
            else if(!(collision&secondMask[q]))d=second[q];
            if(d!=dirs[q])speed=0x100;
        }
        unsigned axisMask=(d==0?0x0006:d==8?0x6000:d==16?0x0060:0x0600);
        int moves=!(collision&axisMask);
        unsigned result=LinearMoveDirectionOLD(&e,0x280,dirs[q]);
        assert(e.x.WORD==0x12345678+dx[d/8]*(int)(speed*256)*moves);
        assert(e.y.WORD==0x23456789+dy[d/8]*(int)(speed*256)*moves);
        assert(result==(moves?(d%16?1:2):0));
    }
    for(unsigned d=0;d<32;d++) {
        Entity e={0};e.x.WORD=0x100000;e.y.WORD=0x200000;
        assert(LinearMoveDirectionOLD(&e,0x180,d));
        int x=e.x.WORD-0x100000,y=e.y.WORD-0x200000;
        assert(x>=-0x18000&&x<=0x18000&&y>=-0x18000&&y<=0x18000);
        Entity opposite={0};LinearMoveDirectionOLD(&opposite,0x180,(d+16)%32);
        assert(opposite.x.WORD==-x&&opposite.y.WORD==-y);
        e.collisions=0x6666;Entity before=e;
        assert(!LinearMoveDirectionOLD(&e,0x180,d));
        assert(!memcmp(&before,&e,sizeof e));
        e.collisions=0;before=e;
        assert(!LinearMoveDirectionOLD(&e,0x180,d|0x80));
        assert(!memcmp(&before,&e,sizeof e));
    }
}
bool DsiRoomCollisionE41_Ready(void){return ready;}
static u16 mask(unsigned shape,unsigned row){return (u16)((0x9365u+shape*271u)^(row*397u));}
bool DsiGbaBus_Read(uint32_t a,void* out,size_t n) {
    u8* b=out;
    if(fault==1)return false;
    if(a==0x08008374&&n==672) {
        for(unsigned k=0;k<7;k++)for(unsigned i=0;i<96;i++)b[k*96+i]=(i+k*3)%40;
        if(fault==2)b[400]=40;
        return true;
    }
    if(a>=0x08000000+PORT_COLLISION_SHAPE_PTRS_EU && a<0x08000000+PORT_COLLISION_SHAPE_PTRS_EU+160 && n==4) {
        u32 p=0x08009000+(a-(0x08000000+PORT_COLLISION_SHAPE_PTRS_EU))*8;
        if(fault==3)p++;
        for(unsigned i=0;i<4;i++)b[i]=p>>(i*8);
        return true;
    }
    if(a>=0x08009000&&a<0x08009500&&n==32) {
        if(fault==4)return false;
        unsigned idx=(a-0x08009000)/32;
        for(unsigned i=0;i<16;i++){u16 v=mask(idx,i);b[2*i]=v;b[2*i+1]=v>>8;}
        return true;
    }
    return false;
}
int main(void) {
    movement_tests();
    Entity e={0};u32 blocked=99;e.collisionLayer=1;
    assert(!DsiCollisionE42_Point(&e,0,0,&blocked)&&blocked==99);
    assert(!DsiCollisionE42_Prepare());ready=true;
    for(fault=1;fault<=4;fault++){DsiCollisionE42_Reset();assert(!DsiCollisionE42_Prepare());}
    fault=0;assert(DsiCollisionE42_Prepare());
    for(unsigned mode=0;mode<7;mode++) {
        memset(&gPlayerState,0,sizeof gPlayerState);
        if(mode==1)gPlayerState.flags=PL_MINISH;
        if(mode==2){gPlayerState.flags=PL_MINISH;gPlayerState.swim_state=1;}
        if(mode==3)gPlayerState.swim_state=1;
        if(mode==4)gPlayerState.jump_status=1;
        if(mode==5)gPlayerState.heldObject=1;
        if(mode==6)gPlayerState.attachedBeetleCount=1;
        for(unsigned collision=0;collision<=255;collision++) {
            gMapBottom.collisionData[0]=collision;
            for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++) {
                if(collision>=112&&collision!=255) {
                    blocked=99;assert(!DsiCollisionE42_Point(&e,x,y,&blocked)&&blocked==99);continue;
                }
                assert(DsiCollisionE42_Point(&e,x,y,&blocked));
                unsigned c=collision,want;
                if(gPlayerState.swim_state&&c<16)c=15;
                if(c==255)want=1;
                else if(c<16)want=(c>>((y<8?2:0)+(x<8?1:0)))&1;
                else want=(mask((c-16+mode*3)%40,y)>>(15-x))&1;
                assert(blocked==want);
            }
        }
    }
    /* Original eight-point hitbox probe: all edges, layer, world origin. */
    memset(&gPlayerState,0,sizeof gPlayerState);
    memset(gMapBottom.collisionData,15,sizeof gMapBottom.collisionData);
    gRoomControls.origin_x=100;gRoomControls.origin_y=200;
    e.x.HALF.HI=132;e.y.HALF.HI=232;
    assert(!DsiCollisionE42_Probe(&e)); /* never installs a fake hitbox */
    e.hitbox=(Hitbox*)&gPlayerHitbox;
    assert(DsiCollisionE42_Probe(&e));assert(e.collisions==0x6666);
    e.collisionLayer=2;assert(DsiCollisionE42_Probe(&e));assert(!e.collisions);
    e.collisionLayer=1;memset(gMapBottom.collisionData,0,sizeof gMapBottom.collisionData);
    const int dx[]={5,5,-5,-5,3,-3,3,-3};
    const int dy[]={3,-3,3,-3,5,5,-5,-5};
    const unsigned bits[]={14,13,10,9,6,5,2,1};
    /* Independently determine bits affected by each blocking metatile. */
    for(unsigned tile=0;tile<256;tile++) {
        gMapBottom.collisionData[tile]=15;unsigned want=0;
        for(unsigned i=0;i<8;i++)if(((32+dy[i]-3)/16)*64+(32+dx[i])/16==(int)tile)want|=1u<<bits[i];
        assert(DsiCollisionE42_Probe(&e));assert(e.collisions==want);
        gMapBottom.collisionData[tile]=0;
    }
    assert(e.x.HALF.HI==132&&e.y.HALF.HI==232);
    /* Genuine movement wrapper refreshes hitbox collisions; the original
       airborne/type2 exclusions retain previously computed collisions. */
    e.direction=8;e.speed=0x180;e.collisions=0x6666;
    sub_0800857C(&e);assert(!e.collisions);assert(e.x.WORD==(132<<16)+0x18000);
    memset(gMapBottom.collisionData,15,sizeof gMapBottom.collisionData);
    Entity before=e;sub_0800857C(&e);
    assert(e.collisions==0x6666&&e.x.WORD==before.x.WORD&&e.y.WORD==before.y.WORD);
    e.collisions=0;e.type2=0x80;sub_0800857C(&e);
    assert(!e.collisions&&e.x.WORD==before.x.WORD+0x18000);
    e.type2=0;gPlayerState.jump_status=0x80;before=e;sub_0800857C(&e);
    assert(!e.collisions&&e.x.WORD==before.x.WORD+0x18000);
    gMapTop.collisionData[4000]=112;
    assert(!DsiCollisionE42_Probe(&e));
    ready=false;assert(!DsiCollisionE42_Ready());
    puts("E42 original pixel collision and movement: modes, masks, 262144 wall-slide cases, fixed-point signs PASS");
}
