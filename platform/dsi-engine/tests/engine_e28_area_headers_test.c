/* E28.1 synthetic-ROM test: no game art, no replacement room algorithm. */
#include "dsi_area_headers.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define BASE 0x08000000u
static uint8_t rom[1024];
static bool failRead;
static void put16(unsigned at, uint16_t n) {
    rom[at]=(uint8_t)n;rom[at+1]=(uint8_t)(n>>8);
}
bool DsiRomRefs_IsRomSpan(uint32_t a,size_t n){
    return n>0u && a>=BASE && a<0x09000000u && n<=0x09000000u-a;
}
bool DsiRomRefs_RomArrayEntry(uint32_t table,uint32_t area,uint32_t *addr) {
    if(table!=0x0811D95Cu || area!=0x22u || !addr)return false;
    *addr=BASE+128u;return true;
}
bool DsiGbaBus_Read(uint32_t a,void *dst,size_t n) {
    if(failRead||!dst||a<BASE)return false;
    uint32_t off=a-BASE;
    if(off>sizeof rom||n>sizeof rom-off)return false;
    memcpy(dst,rom+off,n);return true;
}
int main(void){
    for(unsigned i=0;i<23u;++i){
        const unsigned p=128u+i*10u;
        put16(p,(uint16_t)(i*16u));
        put16(p+2,48u);
        put16(p+4,240u);
        put16(p+6,160u);
        put16(p+8,7u);
    }
    put16(128u+23u*10u,0xFFFFu);
    DsiAreaHeaders_Reset();
    assert(!DsiAreaHeaders_Current());
    assert(DsiAreaHeaders_PrepareEU(0x22u));
    const DsiAreaHeaders *s=DsiAreaHeaders_Current();
    assert(s&&s->ready&&s->count==23u&&s->area==0x22u);
    const RoomHeader *h=DsiAreaHeaders_Get(0x22u,0x15u);
    assert(h&&h->map_x==21u*16u && h->pixel_width==240u);
    RoomHeader old=*h;
    failRead=true;
    assert(!DsiAreaHeaders_PrepareEU(0x22u));
    assert(DsiAreaHeaders_Get(0x22u,0x15u)==h);
    assert(memcmp(&old,h,sizeof old)==0);
    failRead=false;
    assert(!DsiAreaHeaders_PrepareEU(0x99u));
    assert(!DsiAreaHeaders_Get(0x21u,0x15u));
    assert(!DsiAreaHeaders_Get(0x22u,23u));
    DsiAreaHeaders_Reset();
    assert(!DsiAreaHeaders_Current());
    puts("PASS E28.1: original ROM RoomHeader mirror, sentinel and atomic failure");
    return 0;
}
