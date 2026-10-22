/* E35: original EU ROM sprite renderer bounds and transparency, host ASan. */
#include "tmc_link_sprite.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
static TmcLinkSprite link;
static uint16_t frame[256*192+8];
static unsigned changed(void){
 unsigned n=0;for(unsigned i=0;i<256u*192u;i++)
  n+=(frame[i]!=0x1290u);
 return n;
}
static void reset(void){
 for(unsigned i=0;i<256u*192u+8u;i++) frame[i]=0x1290u;
}
int main(void){
 assert(!TmcLink_Load(NULL,NULL));
 link.ready=true;link.frames[0].count=1;
 link.frames[0].pieces[0].flags=0u;
 link.frames[0].pieces[0].tile_lo=0u;
 link.frames[0].pieces[0].tile_hi=0u;
 memset(link.tiles,0x11,32);link.palette[0][1]=0x1234u;
 reset();
 TmcLink_Draw(&link,frame,256,256,192,128,100,0u,0u);
 assert(changed()==64u);
 assert(frame[100*256+128]==0x9234u);
 for(unsigned i=256u*192u;i<256u*192u+8u;i++)assert(frame[i]==0x1290u);
 reset();
 TmcLink_Draw(&link,frame,256,256,192,5,14,0u,0u);
 /* 8x8 object clips 3 columns at x<8 and 2 rows at y<16. */
 assert(changed()==5u*6u);
 reset();
 TmcLink_Draw(&link,frame,256,256,192,128,100,50u,0u);
 TmcLink_Draw(&link,frame,256,256,192,128,100,0u,4u);
 assert(changed()==0u);
 memset(link.tiles,0,32);
 TmcLink_Draw(&link,frame,256,256,192,128,100,0u,0u);
 assert(changed()==0u);
 for(unsigned i=256u*192u;i<256u*192u+8u;i++)assert(frame[i]==0x1290u);
 puts("E35 original Link ROM sprite: pixels, source palette, clipping, transparency PASS");
}
