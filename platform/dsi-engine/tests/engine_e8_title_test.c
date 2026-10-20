#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_title_composite.h"
#include "dsi_gba_bus.h"
#include "dsi_gba_renderer.h"
#include "tmc_rom_stream.h"
#include "port_gba_mem.h"
#include "main.h"
#include "game.h"
#include "common.h"
#include "screen.h"
#include "save.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Input gInput;
Main gMain;
static uint16_t frame[256*192];
static uint32_t hash_visible(void) {
 uint32_t h=2166136261u;
 for(int y=16;y<176;y++) for(int x=8;x<248;x++) {
  uint16_t v=frame[y*256+x];h=(h^(v&255))*16777619u;h=(h^(v>>8))*16777619u;
 }
 return h;
}
int main(int argc,char **argv) {
 assert(argc>=2);
 DsiGbaBus_Init();
 /* Unit-test the compositor before touching the ROM. */
 gBgPltt[0]=0x7fff;gBgPltt[1]=0x001f;gBgPltt[2]=0x03e0;
 gVram[0]=0x21; /* FG first two pixels index 1,2 */
 gVram[32]=0x02; /* BG first pixel index 2 */
 gVram[0x800]=0;gVram[0x801]=0;
 gVram[0x1000]=1;gVram[0x1001]=0;
 DsiTitleCompositeStats st;
 assert(DsiTitleComposite_Draw(frame,256,gVram,sizeof(gVram),gBgPltt,256,
  (1u<<8),0,0,(2u<<8),0,0,8,8,&st));
 assert(frame[16*256+8] == (uint16_t)(0x8000 | 0x01ef)); /* red+green */
 assert(frame[16*256+9] == (uint16_t)(0x8000 | 0x03e0)); /* fg green only */
 assert(st.blended_pixels >= 1);
 puts("PASS E8: two BGs, transparency, GBA alpha blend");
 DsiGbaBus_Init();
 TmcRomReader rom={0};
 assert(TmcRom_Open(&rom,argv[1]));
 assert(rom.region==TMC_ROM_REGION_EUROPE);
 DsiGbaBus_AttachRom(&rom);
 DsiTitleAssets_Enable(true,1);
 DsiTitleAssets_ResetStats();
 assert(DsiTitleTrace_TestFirstTransitions());
 /* 120-frame logo timer, then ORIGINAL TitleTask enters the next scene. */
 for(int i=4;i<=125;i++) TitleTask();
 assert(gUI.lastState==1 && gMain.state==GAMETASK_INIT);
 assert(gDsiTitleTrace.gfx_count==4 && gDsiTitleTrace.gfx_group[2]==2 &&
        gDsiTitleTrace.gfx_group[3]==3);
 assert(gDsiTitleTrace.palette_count==2 && gDsiTitleTrace.palette_group[1]==4);
 assert(gDsiTitleAssetStats.errors==0);
 assert(gScreen.lcd.displayControl & DISPCNT_BG0_ON);
 assert(gScreen.lcd.displayControl & DISPCNT_BG1_ON);
 assert(DsiTitleComposite_Draw(frame,256,gVram,sizeof(gVram),gBgPltt,256,
  gScreen.bg0.control,gScreen.bg0.xOffset,gScreen.bg0.yOffset,
  gScreen.bg1.control,gScreen.bg1.xOffset,gScreen.bg1.yOffset,
  9,9,&st));
 printf("E8 forest: fg=%u bg=%u blend=%u hash=%08x unsupported=%u\n",
   st.foreground_pixels,st.background_pixels,st.blended_pixels,hash_visible(),
   gDsiTitleTrace.unsupported_service_calls);
 assert(st.background_pixels==38400 && st.foreground_pixels==5872);
 assert(st.blended_pixels==5872 && hash_visible()==0x4f749a4au);
 if(argc>=3 && argv[2][0]) {
  FILE *f=fopen(argv[2],"wb");assert(f);
  fprintf(f,"P6\n256 192\n255\n");
  for(int i=0;i<256*192;i++) {uint16_t c=frame[i];unsigned char rgb[3]={
   (c&31)*255/31,((c>>5)&31)*255/31,((c>>10)&31)*255/31};fwrite(rgb,1,3,f);}
  fclose(f);
 }
 puts("PASS E8: original title logo timer advances into EU title background assets");
 puts("NOT YET: logo sprites, real fades, sound, complete AgbMain or playable title");
 return 0;
}
