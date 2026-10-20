#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tmc_link_sprite.h"

static TmcRomReader r;
static TmcLinkSprite link;
static uint16_t image[256 * 192 + 8];
int main(int argc, char **argv) {
    if (argc < 2) { fputs("Usage: link_test /path/to/own/baserom_eu.gba\n", stderr); return 2; }
    const char *path = argv[1];
    if (!TmcRom_Open(&r, path)) {
        fprintf(stderr, "ERROR: Cannot open ROM %s\n", path);
        return 1;
    }
    assert(r.region == TMC_ROM_REGION_EUROPE);
    assert(TmcLink_Load(&link, &r));
    assert(link.frames[0].tile_base == 0);
    assert(link.frames[0].count == 4);
    assert(link.frames[1].tile_base == 7);
    assert(link.frames[27].tile_base > 0);
    for (int f = 0; f < 50; ++f) {
        for (int c = 0; c < 4; ++c) {
            for (unsigned i = 0; i < 256 * 192 + 8; ++i) image[i] = 0x1290;
            TmcLink_Draw(&link, image, 256, 256, 192, 128, 120, (unsigned)f, (unsigned)c);
            int changed = 0;
            for (unsigned i = 0; i < 256 * 192; ++i) changed += image[i] != 0x1290;
            assert(changed >= 30 && changed < 2048);
            for (unsigned i = 256 * 192; i < 256 * 192 + 8; ++i) assert(image[i] == 0x1290);
            if (f == 0 && c == 0) printf("Link frame 0: %d colored pixels\n", changed);
        }
    }
    TmcLink_Draw(&link, image, 256, 256, 192, 128, 120, 50, 0); /* out of range */
    TmcLink_Draw(&link, image, 256, 256, 192, 128, 120, 0, 4); /* out of range */
    TmcRom_Close(&r);
    puts("V4: 50 frames x 4 palettes OK; no sprite bounds overrun");
    return 0;
}
