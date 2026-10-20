/* E27.2: verify stable original 12-byte GBA descriptors in native RAM. */
#include "dsi_room_resources.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define BASE 0x08000000u
static unsigned char raw[0x1000];
static unsigned read_count;
static int broken;
static void put32(unsigned addr, uint32_t value) {
    for(unsigned i=0; i<4; ++i) raw[addr+i] = (unsigned char)(value >> (i*8u));
}
bool DsiGbaBus_Read(uint32_t gba, void *out, size_t size) {
    if (gba < BASE || !out) return false;
    uint32_t offset = gba - BASE;
    if (offset > sizeof raw || size > sizeof raw - offset) return false;
    ++read_count;
    memcpy(out, raw+offset, size);
    return true;
}
bool DsiGbaBus_IsWritable(uint32_t addr, size_t size) {
    return addr >= 0x02000000u && addr < 0x02040000u &&
           size <= 0x02040000u-addr;
}
bool DsiRomRefs_IsRomSpan(uint32_t addr, size_t size) {
    return size && addr >= BASE && addr < 0x09000000u &&
           size <= 0x09000000u-addr;
}
bool DsiRomRefs_RomArrayEntry(uint32_t array, uint32_t index, uint32_t *out) {
    if (!out || broken) return false;
    if (array==0x08101BC8u && index==0x22u) *out=BASE+0x100u;
    else if(array==BASE+0x100u && index==7u) *out=BASE+0x300u;
    else if(array==0x081027F8u && index==0x22u) *out=BASE+0x400u;
    else return false;
    return true;
}
static DsiRoomMetadata metadata(void) {
    DsiRoomMetadata m={0};
    m.area=0x22u; m.room=0x15u; m.width=240u; m.height=160u;
    m.tileset_id=7u; m.room_map_address=BASE+0x500u;
    m.properties_address=BASE+0x700u;
    m.entity_list_addresses[0]=BASE+0x800u;
    m.entity_list_addresses[1]=BASE+0x820u;
    m.entity_counts[0]=3; m.entity_counts[1]=1;
    return m;
}
int main(void) {
    /* Map: 2 original descriptor records with high-bit chaining. */
    put32(0x500, 0x80000020u);
    put32(0x504, 0x02000100u);
    put32(0x508, 32u);
    put32(0x50c, 0x00000080u);
    put32(0x510, 0x02000200u);
    put32(0x514, 16u);
    /* Tileset: one original command. */
    put32(0x300,0x40u);
    put32(0x304,0x02000300u);
    put32(0x308,96u);
    /* Tiles: legal original palette-group command. */
    put32(0x400,5u);
    put32(0x404,0u);
    put32(0x408,0u);
    DsiRoomResources_Reset();
    assert(DsiRoomResources_Current()==NULL);
    DsiRoomMetadata m=metadata();
    assert(DsiRoomResources_PrepareEU(&m));
    const DsiRoomResources *p=DsiRoomResources_Current();
    assert(p && p->valid && p->area==0x22u && p->room==0x15u);
    assert(p->map.count==2u && p->tileset.count==1u && p->tiles.count==1u);
    assert(p->map.command[0].src==0x80000020u);
    assert(p->map.command[0].dest==0x02000100u);
    assert(p->tileset.command[0].size==96u);
    assert(p->tiles.command[0].dest==0u);
    assert(p->entity_counts[0]==3u);
    const DsiRoomResources snapshot=*p;
    /* Invalid chain, length 0, missing terminator and bad references must
       preserve prior snapshot on every failed attempt. */
    put32(0x508,0u);
    assert(!DsiRoomResources_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    put32(0x508,32u);
    broken=1;
    assert(!DsiRoomResources_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    broken=0;
    m.room=64;
    assert(!DsiRoomResources_PrepareEU(&m));
    assert(memcmp(p,&snapshot,sizeof snapshot)==0);
    m=metadata();
    assert(DsiRoomResources_PrepareEU(&m));
    assert(DsiRoomResources_Current()==p);
    assert(read_count>=9);
    DsiRoomResources_Reset();
    assert(DsiRoomResources_Current()==NULL);
    puts("PASS E27.2: native ROM descriptors persistent, bounded and atomic");
    return 0;
}
