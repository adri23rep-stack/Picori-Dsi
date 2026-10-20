#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../source/tmc_gfx_preview.h"

int main(void) {
    TmcGfxPreview p;
    memset(&p, 0, sizeof(p));
    assert(TmcGfxPreview_Pixel(&p, 0, 0, 0) == 0x8000);
    p.ready = true;
    /* Tile 0 pixel [0,0] low nibble; [1,0] high nibble. */
    p.tiles[0] = 0x21;
    p.tiles[1] = 0x43;
    p.palette[0][1] = 0x7c00;
    p.palette[0][2] = 0x03e0;
    p.palette[0][3] = 0x001f;
    p.palette[0][4] = 0x7fff;
    assert(TmcGfxPreview_Pixel(&p, 0, 0, 0) == 0xfc00);
    assert(TmcGfxPreview_Pixel(&p, 1, 0, 0) == 0x83e0);
    assert(TmcGfxPreview_Pixel(&p, 2, 0, 0) == 0x801f);
    assert(TmcGfxPreview_Pixel(&p, 3, 0, 0) == 0xffff);
    /* Tile 1 and row 1 mapping. */
    p.tiles[32] = 0x01;
    p.tiles[4] = 0x02;
    assert(TmcGfxPreview_Pixel(&p, 8, 0, 0) == 0xfc00);
    assert(TmcGfxPreview_Pixel(&p, 0, 1, 0) == 0x83e0);
    assert(TmcGfxPreview_Pixel(&p, 256, 0, 0) == 0x8000);
    assert(TmcGfxPreview_Pixel(&p, 0, 256, 0) == 0x8000);
    assert(TmcGfxPreview_Pixel(&p, 0, 0, 15) == 0x8000);
    puts("gfx 4bpp preview: all unit tests passed");
    return 0;
}
