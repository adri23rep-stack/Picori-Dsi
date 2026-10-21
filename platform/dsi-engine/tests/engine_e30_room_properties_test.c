/* E30.1: synthetic table verifies preservation, invalid pointer refusal,
 * Thumb address classification and atomic failure; no commercial ROM needed.
 */
#include "dsi_room_properties_e30.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BASE 0x08000000u
static unsigned char rom[0x1000];
static bool fail_read;
static unsigned reads;
static void put32(unsigned off, uint32_t v) {
    for (unsigned i=0;i<4u;++i) rom[off+i] = (unsigned char)(v >> (8u*i));
}
bool DsiRomRefs_IsRomSpan(uint32_t addr,size_t size) {
    return size>0u && addr>=BASE && addr<0x09000000u &&
           size<=(size_t)(0x09000000u-addr);
}
bool DsiGbaBus_Read(uint32_t addr,void *out,size_t size) {
    if (fail_read || !out || addr<BASE) return false;
    uint32_t at=addr-BASE;
    if (at>sizeof rom || size>sizeof rom-at) return false;
    ++reads;
    memcpy(out,rom+at,size);
    return true;
}
static DsiRoomMetadata room(void) {
    DsiRoomMetadata r={0};
    r.area=0x22; r.room=0x15;
    r.properties_address=BASE+0x100;
    r.entity_list_addresses[0]=BASE+0x200;
    r.entity_list_addresses[1]=BASE+0x300;
    return r;
}
int main(void) {
    const DsiRoomMetadata m=room();
    put32(0x100,BASE+0x200);
    put32(0x104,BASE+0x300);
    put32(0x108,0u);
    put32(0x10c,BASE+0x400);
    put32(0x110,BASE+0x501); /* GBA Thumb callback, NOT native code */
    put32(0x114,0u);
    put32(0x118,BASE+0x601);
    put32(0x11c,0u);
    DsiRoomPropertiesE30_Reset();
    assert(DsiRoomPropertiesE30_Current()==NULL);
    assert(DsiRoomPropertiesE30_PrepareEU(&m));
    const DsiRoomPropertiesE30 *p=DsiRoomPropertiesE30_Current();
    assert(p && p->area==0x22 && p->room==0x15);
    assert(p->valid && p->gba_table_address==m.properties_address);
    assert(p->kind[0]==DSI_E30_PROPERTY_ROM_DATA &&
           p->kind[1]==DSI_E30_PROPERTY_ROM_DATA);
    assert(p->original[0]==m.entity_list_addresses[0] &&
           p->original[1]==m.entity_list_addresses[1]);
    assert(p->kind[2]==DSI_E30_PROPERTY_NONE &&
           p->kind[3]==DSI_E30_PROPERTY_ROM_DATA);
    assert(p->original[4]==BASE+0x501 &&
           p->kind[4]==DSI_E30_PROPERTY_THUMB_ADDRESS);
    assert(p->kind[6]==DSI_E30_PROPERTY_THUMB_ADDRESS);
    assert(!p->native_dispatch_ready);
    assert(reads>=7u);
    const DsiRoomPropertiesE30 snapshot=*p;
    /* Refuse mismatches while preserving last known-good data. */
    put32(0x104,BASE+0x444);
    assert(!DsiRoomPropertiesE30_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    put32(0x104,BASE+0x300);
    put32(0x118,0x02003000u);
    assert(!DsiRoomPropertiesE30_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    put32(0x118,BASE+0x601);
    fail_read=true;
    assert(!DsiRoomPropertiesE30_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    fail_read=false;
    DsiRoomMetadata unaligned=m;
    unaligned.properties_address++;
    assert(!DsiRoomPropertiesE30_PrepareEU(&unaligned));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    assert(DsiRoomPropertiesE30_PrepareEU(&m));
    DsiRoomPropertiesE30_Reset();
    assert(DsiRoomPropertiesE30_Current()==NULL);
    puts("PASS E30.1: 8 original EU property words pinned, validated and never executed");
    return 0;
}
