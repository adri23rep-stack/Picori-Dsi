/* Host-only E25 tests: synthetic GBA shadow data, NO commercial ROM needed.
 * Checks the 16x16 metatile expansion and basic original 4bpp GBA BG rules.
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "dsi_room_preview.h"

uint8_t gVram[0x18000];
uint16_t gBgPltt[256];
uint8_t gEwram[0x40000];
static uint16_t frame[192][256];
#define BG_MAP  0x02025EB4u
#define TOP_MAP 0x0200B654u
#define BG_META 0x0202CEB4u
#define TOP_META 0x02012654u

bool DsiGbaBus_Read(uint32_t a, void *dst, size_t bytes) {
    if (!dst || a < 0x02000000u || a - 0x02000000u > sizeof gEwram ||
        bytes > sizeof gEwram - (a - 0x02000000u)) return false;
    memcpy(dst, gEwram + a - 0x02000000u, bytes);
    return true;
}
static void write16(uint32_t addr, uint16_t value) {
    gEwram[addr - 0x02000000u] = (uint8_t)value;
    gEwram[addr - 0x02000000u + 1u] = (uint8_t)(value >> 8);
}
static DsiRoomMetadata room(void) {
    DsiRoomMetadata r = {0};
    r.area = 0x22; r.room = 0x15; r.width = 240; r.height = 160;
    return r;
}
int main(void) {
    memset(gEwram, 0, sizeof gEwram);
    gBgPltt[0] = 0;
    gBgPltt[1] = 0x001fu; /* red */
    gBgPltt[2] = 0x03e0u; /* green */
    /* metatile 1 references BG character tile 0, with 4 identical subtiles. */
    for (unsigned i=0; i<4; ++i) write16(BG_META + 8u + i*2u, 0);
    for (unsigned i=0; i<4; ++i) write16(TOP_META + 8u + i*2u, 0);
    for (unsigned i=0; i<150; ++i) {
        write16(BG_MAP + i*2u, 1);
        write16(TOP_MAP + i*2u, 0xffffu); /* top layer empty */
    }
    memset(gVram, 0x11, 32); /* color 1 across bottom bank tile #0 */
    DsiRoomPreviewStats stats = {0};
    DsiRoomMetadata r = room();
    assert(DsiRoomPreview_Draw(&r, &frame[0][0], 256, &stats));
    assert(frame[0][0] == 0x8000);
    assert(frame[16][8] == 0x801f);
    assert(frame[175][247] == 0x801f);
    assert(frame[176][8] == 0x8000);
    assert(stats.colored_pixels == 240u * 160u);
    assert(stats.metatiles == 150u);
    assert(stats.skipped_special == 150u);
    /* Upper-layer character bank = 1, palette 0 color index 2. */
    memset(gVram + 0x4000, 0x22, 32);
    for (unsigned i=0; i<150; ++i) write16(TOP_MAP + i*2u, 1);
    assert(DsiRoomPreview_Draw(&r, &frame[0][0], 256, &stats));
    assert(frame[16][8] == 0x83e0);
    assert(stats.skipped_special == 0);
    /* Transparent pixels on upper layer reveal the lower layer. */
    memset(gVram + 0x4000, 0, 32);
    assert(DsiRoomPreview_Draw(&r, &frame[0][0], 256, &stats));
    assert(frame[16][8] == 0x801f);
    assert(stats.transparent_pixels > 0);
    /* A bad area is rejected BEFORE framebuffer modification. */
    uint16_t before = frame[16][8];
    r.area = 0x91;
    assert(!DsiRoomPreview_Draw(&r, &frame[0][0], 256, &stats));
    assert(frame[16][8] == before);
    /* Insufficient output stride is rejected. */
    r = room();
    assert(!DsiRoomPreview_Draw(&r, &frame[0][0], 255, &stats));
    /* Clipped oversized room cannot read off the 64x64 map. */
    r.width = 1024; r.height = 1024;
    assert(DsiRoomPreview_Draw(&r, &frame[0][0], 256, &stats));
    puts("E25 preview: PASS (16x16 tiles / bank BG1 / alpha / bounds)");
    return 0;
}
