/* E28.3 regression using fake EU ROM records, no copyrighted data. */
#include "dsi_entity_lists.h"
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define BASE 0x08000000u
static uint8_t rom[1024];
static bool fail_read;
bool DsiRomRefs_IsRomSpan(uint32_t address, size_t size) {
    return size > 0u && address >= BASE && address < 0x09000000u &&
           size <= 0x09000000u - address;
}
bool DsiGbaBus_Read(uint32_t address, void *dst, size_t size) {
    if (fail_read || !dst || address < BASE) return false;
    uint32_t at = address - BASE;
    if (at > sizeof rom || size > sizeof rom - at) return false;
    memcpy(dst, rom + at, size);
    return true;
}
int main(void) {
    DsiRoomMetadata room = {0};
    room.area = 0x22u;
    room.room = 0x15u;
    room.entity_list_addresses[0] = BASE + 0x100u;
    room.entity_list_addresses[1] = BASE + 0x200u;
    room.entity_counts[0] = 2;
    room.entity_counts[1] = 1;
    /* Exactly original 16-byte records terminated by first-byte 0xFF. */
    rom[0x100] = 1; rom[0x101] = 0xAB;
    rom[0x110] = 3; rom[0x111] = 0xCD;
    rom[0x120] = 0xFF;
    rom[0x200] = 7; rom[0x201] = 0xEF;
    rom[0x210] = 0xFF;
    DsiEntityLists_Reset();
    assert(DsiEntityLists_Current() == NULL);
    assert(DsiEntityLists_PrepareEU(&room));
    const DsiEntityLists *ptr = DsiEntityLists_Current();
    assert(ptr && ptr->valid && ptr->area == 0x22u && ptr->room == 0x15u);
    assert(ptr->count[0] == 2 && ptr->count[1] == 1);
    assert(ptr->raw[0][0][1] == 0xAB && ptr->raw[0][1][1] == 0xCD);
    assert(ptr->raw[1][0][1] == 0xEF && ptr->raw[1][1][0] == 0xFF);
    /* New source reads do not mutate old stable native list. */
    rom[0x101] = 0x11;
    assert(ptr->raw[0][0][1] == 0xAB);
    fail_read = true;
    assert(!DsiEntityLists_PrepareEU(&room));
    assert(DsiEntityLists_Current() == ptr && ptr->raw[0][0][1] == 0xAB);
    fail_read = false;
    /* Reject early sentinel inconsistent with the room probe count. */
    rom[0x110] = 0xFF;
    assert(!DsiEntityLists_PrepareEU(&room));
    assert(ptr->raw[0][1][0] == 3);
    rom[0x110] = 3;
    room.entity_counts[0] = 128;
    assert(!DsiEntityLists_PrepareEU(&room));
    room.entity_counts[0] = 2;
    room.entity_list_addresses[0] = BASE + 1020u;
    assert(!DsiEntityLists_PrepareEU(&room));
    room.entity_list_addresses[0] = BASE + 0x100u;
    assert(DsiEntityLists_PrepareEU(&room));
    assert(ptr->raw[0][0][1] == 0x11);
    DsiEntityLists_Reset();
    assert(DsiEntityLists_Current() == NULL);
    puts("PASS E28.3: original entity record bytes stable, bounded and atomic");
    return 0;
}
