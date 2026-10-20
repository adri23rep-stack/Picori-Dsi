#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_title_composite.h"
#include "dsi_title_live.h"
#include "dsi_title_sprites.h"
#include "dsi_title_entities.h"
#include "dsi_gba_bus.h"
#include "dsi_gba_renderer.h"
#include "tmc_rom_stream.h"
#include "port_gba_mem.h"
#include "main.h"
#include "object.h"
#include "port_rng.h"
extern u32 Random(void);
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
 DsiTitleSprites_Enable(true);
 DsiTitleEntities_Enable(true);
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
 /* E9: no app-level rendering logic is emulated. All subsequent changes
  * are produced by each original TitleTask() call. */
 DsiTitleLive live; DsiTitleLive_Init(&live);
 bool updated = false;
 assert(DsiTitleLive_Render(&live,frame,256,&updated) && updated);
 uint32_t first = hash_visible(), last = first;
 unsigned distinct = 0, changed_frames = 0, idle_frames = 0;
 for(int tick=0;tick<170;tick++) {
  TitleTask();
  assert(gDsiTitleAssetStats.errors==0);
  updated = false;
  assert(DsiTitleLive_Render(&live,frame,256,&updated));
  if(updated) {
    ++changed_frames;
    uint32_t now=hash_visible();
    if(now!=last) ++distinct;
    last=now;
  } else ++idle_frames;
 }
 printf("E9 rays: redraws=%u palette=%u alpha=%u hashes_changed=%u idle=%u\n",
   live.draws,live.palette_updates,live.alpha_updates,distinct,idle_frames);
 assert(gUI.lastState==1);
 assert(live.draws>=10 && live.draws < 80);
 assert(live.palette_updates>=10 && live.alpha_updates>=2);
 assert(distinct>=3 && first!=last && idle_frames>=100);
 assert(gDsiTitleAssetStats.errors==0);
 assert(DsiGbaBus_InvalidReads()==0 && DsiGbaBus_InvalidWrites()==0);
 puts("PASS E9: original TitleTask animates EU palette cycling and blending");
 if(argc>=3 && argv[2][0]) {
  FILE *f=fopen(argv[2],"wb");assert(f);
  fprintf(f,"P6\n256 192\n255\n");
  for(int i=0;i<256*192;i++) {uint16_t c=frame[i];unsigned char rgb[3]={
   (c&31)*255/31,((c>>5)&31)*255/31,((c>>10)&31)*255/31};fwrite(rgb,1,3,f);}
  fclose(f);
 }
 puts("PASS E9: E8 graphics preserved; E9 original title-ray animation verified");
 /* E10: real TitleTask starts requesting frame-OBJ commands after the title settles. */
 unsigned prior_calls=gDsiTitleSpriteStats.draw_calls, changed_sprites=0;
 uint32_t prior_sig=DsiTitleSprites_Signature();
 for(int step=0;step<560;step++) {
  TitleTask();
  updated=false;
  assert(DsiTitleLive_Render(&live,frame,256,&updated));
  assert(gDsiTitleAssetStats.errors==0 && gDsiTitleSpriteStats.errors==0);
  uint32_t sig=DsiTitleSprites_Signature();
  if(sig!=prior_sig) ++changed_sprites;
  prior_sig=sig;
 }
 printf("E11 entities: created=%u updates=%u live=%u root=%u child=%u unsupported=%u errors=%u\n", gDsiTitleEntityStats.created,gDsiTitleEntityStats.updated,gDsiTitleEntityStats.alive,gDsiTitleEntityStats.root_updates,gDsiTitleEntityStats.child_updates,gDsiTitleEntityStats.unsupported,gDsiTitleEntityStats.errors);
 printf("E10 original OBJ calls=%u pieces=%u sprite changes=%u visible=%u\n",
   gDsiTitleSpriteStats.draw_calls-prior_calls,gDsiTitleSpriteStats.pieces,
   changed_sprites,gDsiTitleSpriteStats.rendered_pixels);
 printf("E10 frame cache hits=%u misses=%u\n",gDsiTitleSpriteStats.cache_hits,gDsiTitleSpriteStats.cache_misses);
 /* Original TitleScreenObject_Type0 creates three children; the actual
  * original object function runs every title tick after the first spawn. */
 assert(gDsiTitleEntityStats.created==4 && gDsiTitleEntityStats.alive==4);
 assert(gDsiTitleEntityStats.subtype_counts[0]==1);
 assert(gDsiTitleEntityStats.subtype_counts[1]==1);
 assert(gDsiTitleEntityStats.subtype_counts[3]==1);
 assert(gDsiTitleEntityStats.subtype_counts[4]==1);
 assert(gDsiTitleEntityStats.subtype_counts[2]==0);
 assert(gDsiTitleEntityStats.updated>100 && gDsiTitleEntityStats.root_updates>0);
 assert(gDsiTitleEntityStats.child_updates>0 && gDsiTitleEntityStats.errors==0);
 assert(gDsiTitleSpriteStats.cache_hits>100 && gDsiTitleSpriteStats.cache_misses>0);
 assert(gDsiTitleSpriteStats.draw_calls>100 && gDsiTitleSpriteStats.pieces>100);
 assert(changed_sprites>0 && gDsiTitleSpriteStats.errors==0);
 assert(gMain.task==TASK_TITLE && gUI.lastState==1);
 /* Save the actual BG image. The current title object graphics are still
  * not installed by the missing original CreateObject/GFX path. */
 if(argc>=3 && argv[2][0]) {
  FILE *f=fopen(argv[2],"wb");assert(f);
  fprintf(f,"P6\n256 192\n255\n");
  for(int i=0;i<256*192;i++){uint16_t c=frame[i];unsigned char rgb[3]={
    (c&31)*255/31,((c>>5)&31)*255/31,((c>>10)&31)*255/31};fwrite(rgb,1,3,f);}
  fclose(f);
 }
 /* Test the DSi OBJ pixel transport with a ROM frame and deliberately
  * generated one-pixel texture. Not a game image. */
 DsiTitleSprites_Clear();
 gOamCmd.x=120;gOamCmd.y=80;gOamCmd._4=0;gOamCmd._6=0;gOamCmd._8=0xE100;
 assert(DsiTitleSprites_DrawDirect(510,1,&gOamCmd));
 assert(DsiTitleSprites_Count()==4);
 memset(frame,0,sizeof(frame));
 memset(gVram+0x10000,0,0x8000);
 memset(gObjPltt,0,sizeof(gObjPltt));
 gVram[0x10000+256*32]=0x11;
 gObjPltt[14*16+1]=0x001f;
 unsigned painted=0;
 assert(DsiTitleSprites_Composite(frame,256,gVram,sizeof(gVram),
        gObjPltt,256,DISPCNT_OBJ_ON,&painted));
 printf("E10 synthetic OBJ pixels=%u sample=%04x\n",painted,frame[(72+16)*256+56+8]);
 assert(painted>0 && frame[(72+16)*256+56+8]==0x801fu);
 assert(!DsiTitleSprites_DrawDirect(511,0,&gOamCmd));
 assert(gDsiTitleSpriteStats.errors==1); /* intentional rejection */
 puts("PASS E10: bounded original OAM lookup, title DrawDirect calls, CPU pixels");
 /* Run a second, isolated title-object spawn with EU alternative flag.
  * The ORIGINAL TitleScreenObject_Type0 branches on EWRAM byte 7. */
 DsiTitleEntities_Clear();
 gba_write8(0x02000007u,1);
 assert(CreateObject(TITLE_SCREEN_OBJECT,0,0)!=NULL);
 DsiTitleEntities_Update();
 assert(gDsiTitleEntityStats.created==4);
 assert(gDsiTitleEntityStats.subtype_counts[0]==1);
 assert(gDsiTitleEntityStats.subtype_counts[1]==1);
 assert(gDsiTitleEntityStats.subtype_counts[2]==1);
 assert(gDsiTitleEntityStats.subtype_counts[3]==0);
 assert(gDsiTitleEntityStats.subtype_counts[4]==1);
 assert(gDsiTitleEntityStats.updated==1); /* snapshot semantics */
 DsiTitleEntities_Update();
 assert(gDsiTitleEntityStats.updated==5);
 /* Only title entities accepted; bounded pool cannot overflow. */
 assert(CreateObject(TITLE_SCREEN_OBJECT+1,0,0)==NULL);
 assert(gDsiTitleEntityStats.unsupported==1);
 for(int n=0;n<4;n++) assert(CreateObject(TITLE_SCREEN_OBJECT,1,0));
 assert(gDsiTitleEntityStats.alive==8);
 assert(CreateObject(TITLE_SCREEN_OBJECT,1,0)==NULL);
 assert(gDsiTitleEntityStats.errors==1);
 DsiTitleEntities_Enable(false);
 assert(CreateObject(TITLE_SCREEN_OBJECT,0,0)==NULL);
 DsiTitleEntities_Enable(true); /* reseeds the real GBA RNG sequence */
 static const uint32_t golden[8]={
   0x40D40DA7u,0x47AA13E1u,0x5D1AB7F1u,0x3EA4BA81u,
   0x7C19DF71u,0x729BA26Cu,0x3A2EBE97u,0x5E297461u
 };
 for(int n=0;n<8;n++) assert(Random()==golden[n]);
 puts("PASS E11: original titleScreenObject.c, child spawning, update, pool bounds");

 puts("NOT YET: original title object's GFX, real fades, sound, AgbMain, gameplay");
 return 0;
}
