/* The Minish Cap / DSi - V4 real ROM metasprite overlay demo.
 * The forest is still the raw v3 title tile SHEET, NOT a playable game map.
 * Link is the authentic EU ROM sprite, assembled using the ROM frame list.
 * Only experimental movement/animation; no game physics, maps or engine. */
#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "tmc_rom_stream.h"
#include "tmc_gfx_preview.h"
#include "tmc_link_sprite.h"

#define TOP_W 256
#define TOP_H 192
#define GBA_W 240
#define GBA_H 160

static PrintConsole sConsole;
static TmcRomReader sRom;
static TmcGfxPreview sGfx;
static TmcLinkSprite sLink;
static uint16_t *sTop;
static uint16_t sBackdrop[TOP_W * TOP_H];
static int sLinkX = 128, sLinkY = 123;
static unsigned sAtlasPanX, sAtlasPanY, sAtlasPalette, sTunic;
static unsigned sFacing = 0, sWalkFrames;
static bool sAtlasView;

static uint16_t Opaque(int r, int g, int b) {
    return (uint16_t)(RGB15(r, g, b) | BIT(15));
}
static void Rect(int x, int y, int w, int h, uint16_t color) {
    for (int yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= TOP_H) continue;
        for (int xx = x; xx < x + w; ++xx)
            if (xx >= 0 && xx < TOP_W) sBackdrop[yy * TOP_W + xx] = color;
    }
}
static void MakeBackdrop(void) {
    for (unsigned i = 0; i < TOP_W * TOP_H; ++i)
        sBackdrop[i] = Opaque(2, 4, 9);
    Rect(4, 12, 248, 168, Opaque(22, 19, 9));
    Rect(8, 16, GBA_W, GBA_H, Opaque(3, 11, 9));
    if (!sGfx.ready) return;
    for (unsigned y = 0; y < GBA_H; ++y) {
        for (unsigned x = 0; x < GBA_W; ++x) {
            sBackdrop[(y + 16u) * TOP_W + (x + 8u)] =
                TmcGfxPreview_Pixel(&sGfx, x + sAtlasPanX,
                                     y + sAtlasPanY, sAtlasView ? sAtlasPalette : 0u);
        }
    }
}
static void Display(unsigned frame) {
    memcpy(sTop, sBackdrop, sizeof(sBackdrop));
    if (!sAtlasView && sLink.ready) {
        TmcLink_Draw(&sLink, sTop, TOP_W, TOP_W, TOP_H,
                     sLinkX, sLinkY, frame, sTunic);
    }
}
static bool FindRom(void) {
    static const char *paths[] = {
        "fat:/tmc-dsi/baserom_eu.gba", "fat:/tmc-dsi/baserom.gba",
        "sd:/tmc-dsi/baserom_eu.gba", "sd:/tmc-dsi/baserom.gba",
        "/tmc-dsi/baserom_eu.gba", "/tmc-dsi/baserom.gba"
    };
    for (unsigned i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        if (TmcRom_Open(&sRom, paths[i])) {
            iprintf("ROM: %s\n", paths[i]);
            return true;
        }
    }
    return false;
}
static unsigned PickFrame(bool moving) {
    /* Frames from gSpriteFrames_1, animated as a short demo walk cycle.
     * Not the original animation scripts yet. */
    static const uint8_t f[4][4] = {
        { 0, 3, 4, 7 },   /* down */
        { 1, 5, 6, 8 },   /* right */
        { 17, 20, 21, 22 }, /* left */
        { 27, 30, 31, 32 } /* up */
    };
    return f[sFacing][moving ? (sWalkFrames / 8u) % 4u : 0u];
}
int main(void) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    sTop = (uint16_t *)bgGetGfxPtr(bg);
    videoSetModeSub(MODE_0_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(&sConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    consoleSelect(&sConsole);
    MakeBackdrop();
    Display(0);
    iprintf("TMC / DSi - LINK V4\n");
    iprintf("========================\n");
    iprintf("System: %s mode\n", isDSiMode() ? "DSi" : "DS");
    bool storage = fatInitDefault();
    iprintf("SD/FAT: %s\n", storage ? "ready" : "FAILED");
    bool loaded = storage && FindRom();
    if (loaded) {
        iprintf("Region: %s\n", sRom.region == TMC_ROM_REGION_EUROPE ? "EU" : "USA");
        iprintf("Forest: %s\n", TmcGfxPreview_Load(&sGfx, &sRom) ? "LOADED" : "FAILED");
        iprintf("Link sprite: %s\n", TmcLink_Load(&sLink, &sRom) ? "LOADED" : "FAILED (EU only)");
    } else {
        iprintf("Missing: /tmc-dsi/\n");
        iprintf("baserom_eu.gba\n");
    }
    iprintf("------------------------\n");
    iprintf("Real Link frames from ROM\n");
    iprintf("Demo only - not the game!\n");
    iprintf("\n");
    iprintf("D-pad: move Link\n");
    iprintf("A: change tunic color\n");
    iprintf("Hold B: move faster\n");
    iprintf("SELECT: v3 tile atlas\n");
    iprintf("START: exit\n");
    MakeBackdrop();
    Display(PickFrame(false));

    unsigned long frames = 0;
    uint32_t prevHeld = 0;
    bool moved = false;
    while (pmMainLoop()) {
        swiWaitForVBlank();
        scanKeys();
        uint32_t apiHeld = keysHeld();
        uint16_t reg = REG_KEYINPUT & 0x03FFu;
        uint32_t held = apiHeld | ((uint32_t)(~reg) & 0x03FFu);
        uint32_t pressed = held & ~prevHeld;
        prevHeld = held;
        if (held & KEY_START) break;
        bool changed = false, newBackdrop = false;
        if (pressed & KEY_SELECT) {
            sAtlasView = !sAtlasView;
            sAtlasPanX = sAtlasPanY = 0;
            changed = newBackdrop = true;
        }
        if (pressed & KEY_A) {
            if (sAtlasView) sAtlasPalette = (sAtlasPalette + 1u) % TMC_TITLE_PALETTE_COUNT;
            else sTunic = (sTunic + 1u) % 4u;
            changed = true;
            if (sAtlasView) newBackdrop = true;
        }
        if (sAtlasView && sGfx.ready) {
            if ((held & KEY_RIGHT) && sAtlasPanX < 16u) { ++sAtlasPanX; changed = newBackdrop = true; }
            if ((held & KEY_LEFT) && sAtlasPanX > 0u) { --sAtlasPanX; changed = newBackdrop = true; }
            if ((held & KEY_DOWN) && sAtlasPanY < 96u) { ++sAtlasPanY; changed = newBackdrop = true; }
            if ((held & KEY_UP) && sAtlasPanY > 0u) { --sAtlasPanY; changed = newBackdrop = true; }
        } else if (sLink.ready) {
            int dx = !!(held & KEY_RIGHT) - !!(held & KEY_LEFT);
            int dy = !!(held & KEY_DOWN) - !!(held & KEY_UP);
            moved = dx || dy;
            if (moved) {
                if (dy < 0) sFacing = 3;
                else if (dy > 0) sFacing = 0;
                else if (dx < 0) sFacing = 2;
                else sFacing = 1;
                int speed = (held & KEY_B) ? 2 : 1;
                sLinkX += dx * speed; sLinkY += dy * speed;
                if (sLinkX < 27) sLinkX = 27;
                if (sLinkX > 228) sLinkX = 228;
                if (sLinkY < 40) sLinkY = 40;
                if (sLinkY > 171) sLinkY = 171;
                ++sWalkFrames;
                changed = true;
            } else if (sWalkFrames != 0) {
                sWalkFrames = 0;
                changed = true;
            }
        }
        if (newBackdrop) MakeBackdrop();
        if (changed) Display(PickFrame(!sAtlasView && moved));
        ++frames;
        if ((frames % 12u) == 0u) {
            iprintf("\x1b[19;0HMode: %-25s", sAtlasView ? "Atlas" : (sLink.ready ? "Walking Link" : "No sprite"));
            iprintf("\x1b[20;0HLink: %03d,%03d Dir:%u  ", sLinkX, sLinkY, sFacing);
            iprintf("\x1b[21;0HTunic:%u Frm:%02u            ", sTunic, PickFrame(moved));
            iprintf("\x1b[22;0HFrames:%-10lu         ", frames);
        }
    }
    TmcRom_Close(&sRom);
    return 0;
}
