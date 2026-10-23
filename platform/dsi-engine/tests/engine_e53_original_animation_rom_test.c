/* E53: synthetic ROM POINTER TABLE test. Never bundle or upload game data.
 * Proves validation and byte ordering; not gameplay or real-ROM verification. */
#include "dsi_player_anim_rom_e53.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#define BASE 0x08000000u
static uint8_t rom[0x10000];
static bool span(uint32_t addr, size_t len) {
    return addr>=BASE && addr-BASE <= sizeof(rom) &&
           len <= sizeof(rom)-(addr-BASE);
}
bool DsiRomRefs_IsRomSpan(uint32_t addr,size_t len) {return span(addr,len);}
bool DsiGbaBus_Read(uint32_t addr,void *out,size_t len) {
    if (!span(addr,len)) return false;
    memcpy(out,rom+(addr-BASE),len);return true;
}
bool DsiRomRefs_ReadU32(uint32_t addr,uint32_t *out) {
    uint8_t x[4];
    if(!out || !DsiGbaBus_Read(addr,x,4))return false;
    *out=(uint32_t)x[0]|(uint32_t)x[1]<<8|
         (uint32_t)x[2]<<16|(uint32_t)x[3]<<24;return true;
}
static void wr32(unsigned o, uint32_t v) {
    rom[o]=(uint8_t)v;rom[o+1]=(uint8_t)(v>>8);
    rom[o+2]=(uint8_t)(v>>16);rom[o+3]=(uint8_t)(v>>24);
}
int main(void) {
    const unsigned entry=0x2A5Cu+16u;
    DsiPlayerAnimE53 info;
    wr32(entry,BASE+0x3400);wr32(entry+4,BASE+0x3500);
    wr32(entry+8,BASE+0x3600);
    wr32(0x3400+2*4,BASE+0x3700);
    rom[0x3700]=7;rom[0x3701]=5;rom[0x3702]=2;rom[0x3703]=0x80;
    assert(DsiPlayerAnimE53_Preflight(1,2,&info));
    assert(info.animations_gba==BASE+0x3400);
    assert(info.animation_gba==BASE+0x3700);
    assert(info.frame_index==7 && info.duration==5);
    assert(info.sprite_settings==2 && info.flags==0x80);
    assert(!DsiPlayerAnimE53_Preflight(9999,2,&info));
    assert(!DsiPlayerAnimE53_Preflight(1,2,NULL));
    wr32(0x3400+2*4,0x07000000u);
    assert(!DsiPlayerAnimE53_Preflight(1,2,&info));
    wr32(0x3400+2*4,BASE+0xFFFFu);
    assert(!DsiPlayerAnimE53_Preflight(1,2,&info));
    wr32(entry+8,0u);
    assert(!DsiPlayerAnimE53_Preflight(1,2,&info));
    return 0;
}
