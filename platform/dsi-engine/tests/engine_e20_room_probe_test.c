#include "dsi_room_probe.h"
#include "dsi_gba_bus.h"
#include "tmc_rom_stream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
    assert(argc == 2);
    TmcRomReader r = {0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&r, argv[1]));
    assert(r.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&r);
    DsiRoomMetadata meta = {0};
    assert(DsiRoomProbe_ReadEU(0x22, 0x15, &meta));
    assert(meta.area == 0x22 && meta.room == 0x15);
    assert(meta.map_x == 0x510 && meta.map_y == 0x1b0);
    assert(meta.width == 240 && meta.height == 160);
    assert(meta.tileset_id == 0);
    assert(meta.room_map_address == 0x08103d50u);
    assert(meta.properties_address == 0x080f2854u);
    assert(meta.entity_list_addresses[0] == 0x080f27bcu);
    assert(meta.entity_list_addresses[1] == 0x080f281cu);
    assert(meta.entity_counts[0] == 3 && meta.entity_counts[1] == 1);
    unsigned invalid_before = DsiGbaBus_InvalidReads();
    assert(!DsiRoomProbe_ReadEU(0xFF, 0, &meta));
    assert(DsiGbaBus_InvalidReads() == invalid_before);
    assert(meta.width == 240); /* failure never publishes partial state */
    assert(!DsiRoomProbe_ReadEU(0x22, 0xFF, &meta));
    assert(meta.width == 240);
    assert(!DsiRoomProbe_ReadEU(0x22, 0x15, NULL));
    DsiGbaBus_DetachRom();
    assert(!DsiRoomProbe_ReadEU(0x22, 0x15, &meta));
    assert(meta.width == 240);
    TmcRom_Close(&r);
    printf("PASS E20 room header 0x22/0x15: %ux%u at (%u,%u), entity lists %u+%u, ROM refs validated, fail closed\n",
           meta.width, meta.height, meta.map_x, meta.map_y,
           meta.entity_counts[0], meta.entity_counts[1]);
    return 0;
}
