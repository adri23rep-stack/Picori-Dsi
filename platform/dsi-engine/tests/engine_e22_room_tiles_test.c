#include "dsi_room_tiles.h"
#include "dsi_gba_bus.h"
#include "dsi_title_assets.h"
#include "tmc_rom_stream.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t checksum(uint32_t address, size_t count) {
    uint8_t bytes[256];
    uint32_t h = 2166136261u;
    while (count) {
        size_t n = count < sizeof bytes ? count : sizeof bytes;
        assert(DsiGbaBus_Read(address, bytes, n));
        for (size_t i = 0; i < n; ++i) h = (h ^ bytes[i]) * 16777619u;
        address += (uint32_t)n;
        count -= n;
    }
    return h;
}
int main(int argc, char **argv) {
    assert(argc == 2);
    TmcRomReader reader = {0};
    DsiGbaBus_Init();
    assert(TmcRom_Open(&reader, argv[1]));
    assert(reader.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&reader);
    DsiTitleAssets_Enable(true, 2);
    DsiTitleAssets_ResetStats();
    DsiRoomMetadata room;
    assert(DsiRoomProbe_ReadEU(0x22, 0x15, &room));
    assert(room.tileset_id == 0);
    DsiRoomTileStats stats = {0};
    assert(DsiRoomTiles_LoadEU(&room, &stats));
    assert(stats.tileset.descriptors == 4 && stats.tileset.lz_blocks == 3);
    assert(stats.tileset.palette_groups == 1 && stats.tileset.uncompressed_bytes == 49152);
    assert(stats.tiles.descriptors == 4 && stats.tiles.lz_blocks == 4);
    assert(stats.tiles.palette_groups == 0 && stats.tiles.uncompressed_bytes == 36980);
    /* Independently decoded GBA LZ77 expected output hashes (not hashes of compressed input). */
    const struct { uint32_t addr; size_t len; uint32_t expected; } blocks[] = {
        {0x06000000u,16384,0x1e80ce5eu}, {0x06004000u,16384,0x12142a43u},
        {0x06008000u,16384,0xd09f78dfu}, {0x0202ceb4u,13216,0x48ff0121u},
        {0x02012654u,16368,0x0922800au}, {0x0202aeb4u,3304,0x695daa1fu},
        {0x02010654u,4092,0xff708692u},
    };
    for (unsigned i = 0; i < sizeof(blocks)/sizeof(blocks[0]); ++i) {
        uint32_t hash = checksum(blocks[i].addr, blocks[i].len);
        printf("E22 block %u at %08x len %zu: %08x\n",i,blocks[i].addr,blocks[i].len,hash);
        assert(hash == blocks[i].expected);
    }
    assert(gDsiTitleAssetStats.palette_groups == 1);
    assert(gDsiTitleAssetStats.errors == 0);
    uint32_t before = checksum(0x06000000u, 16384);
    DsiRoomMetadata bad = room;
    bad.tileset_id = 128;
    assert(!DsiRoomTiles_LoadEU(&bad, &stats));
    assert(checksum(0x06000000u,16384) == before);
    bad = room;
    bad.area = 0xff;
    assert(!DsiRoomTiles_LoadEU(&bad, &stats));
    assert(checksum(0x06000000u,16384) == before);
    assert(!DsiRoomTiles_LoadEU(NULL,&stats));
    assert(!DsiRoomTiles_LoadEU(&room,NULL));
    DsiGbaBus_DetachRom();
    TmcRom_Close(&reader);
    puts("PASS E22 original EU tileset+metatile records, 7 LZ blocks, palette, hashes and bounds");
    return 0;
}
