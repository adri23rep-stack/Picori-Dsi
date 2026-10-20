#include "dsi_room_maps.h"
#include "dsi_gba_bus.h"
#include "tmc_rom_stream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* These expected hashes are independently derived from GBA LZ77 blocks
 * found by following the retail EU MapDataDefinition table. */
static uint32_t fnv1a(const uint8_t *bytes, unsigned n) {
    uint32_t h = 2166136261u;
    for (unsigned i=0; i<n; ++i) h = (h ^ bytes[i]) * 16777619u;
    return h;
}
int main(int argc, char **argv) {
    assert(argc == 2);
    TmcRomReader reader = {0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&reader, argv[1]));
    assert(reader.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&reader);
    DsiRoomMetadata room = {0};
    assert(DsiRoomProbe_ReadEU(0x22, 0x15, &room));
    DsiRoomMapStats stats = {0};
    assert(DsiRoomMaps_LoadEU(&room, &stats));
    assert(stats.descriptors == 2 && stats.lz_blocks == 2);
    assert(stats.uncompressed_bytes == 600);
    uint8_t bottom[300], top[300];
    assert(DsiGbaBus_Read(0x02025eb4u, bottom, sizeof bottom));
    assert(DsiGbaBus_Read(0x0200b654u, top, sizeof top));
    printf("E21 map bottom FNV=%08x top=%08x\n", fnv1a(bottom,300),fnv1a(top,300));
    assert(fnv1a(bottom, 300) == 0xeb1ef36bu);
    assert(fnv1a(top, 300) == 0x6b637c85u);
    DsiRoomMetadata invalid = room;
    invalid.room_map_address = 0x08fffffcu;
    uint8_t before[300];
    memcpy(before, bottom, sizeof before);
    assert(!DsiRoomMaps_LoadEU(&invalid, &stats));
    assert(DsiGbaBus_Read(0x02025eb4u, bottom, sizeof bottom));
    assert(memcmp(before,bottom,sizeof bottom)==0);
    assert(!DsiRoomMaps_LoadEU(NULL,&stats));
    assert(!DsiRoomMaps_LoadEU(&room,NULL));
    DsiGbaBus_DetachRom();
    TmcRom_Close(&reader);
    puts("PASS E21 2 genuine EU room map layers, LZ77, shadow RAM, bounds");
    return 0;
}
