#include "dsi_title_live.h"
#include "dsi_gba_renderer.h"
#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_gba_bus.h"
#include "dsi_title_sprites.h"
#include "main.h"
#include "screen.h"
#include "game.h"
#include <string.h>

void DsiTitleLive_Init(DsiTitleLive *s) { if (s) memset(s, 0, sizeof(*s)); }

bool DsiTitleLive_Render(DsiTitleLive *s, uint16_t *frame, size_t stride, bool *changed) {
    if (changed) *changed = false;
    if (!s || !frame || stride < DSI_RENDER_W) return false;
    if (gUI.lastState != 1) return true;
    if (gDsiTitleAssetStats.errors != 0) return false;
    uint16_t alpha = gScreen.controls.alphaBlend;
    uint16_t fg = gScreen.bg0.control, bg = gScreen.bg1.control;
    int16_t fx = gScreen.bg0.xOffset, fy = gScreen.bg0.yOffset;
    int16_t bx = gScreen.bg1.xOffset, by = gScreen.bg1.yOffset;
    unsigned gfx = gDsiTitleTrace.gfx_count, pal = gDsiTitleTrace.palette_count;
    uint32_t sprites = DsiTitleSprites_Signature();
    bool different = !s->active || alpha != s->last_alpha || fg != s->last_fg_control ||
        bg != s->last_bg_control || fx != s->last_fg_x || fy != s->last_fg_y ||
        bx != s->last_bg_x || by != s->last_bg_y || gfx != s->last_gfx_count ||
        pal != s->last_pal_count || sprites != s->last_sprite_signature;
    if (!different) return true;
    unsigned eva = alpha & 31u, evb = (alpha >> 8) & 31u;
    if (eva > 16u) eva = 16u;
    if (evb > 16u) evb = 16u;
    DsiTitleCompositeStats stats;
    if (!DsiTitleComposite_Draw(frame, stride, gVram, sizeof(gVram), gBgPltt, 256,
        fg, fx, fy, bg, bx, by, eva, evb, &stats)) return false;
    if (!DsiTitleSprites_Composite(frame, stride, gVram, sizeof(gVram),
        gObjPltt, 256, gScreen.lcd.displayControl, NULL)) return false;
    if (s->active) {
        s->palette_updates += pal != s->last_pal_count;
        s->gfx_updates += gfx != s->last_gfx_count;
        s->alpha_updates += alpha != s->last_alpha;
    }
    s->active = true;
    s->last_alpha = alpha;
    s->last_fg_control = fg;
    s->last_bg_control = bg;
    s->last_fg_x = fx;
    s->last_fg_y = fy;
    s->last_bg_x = bx;
    s->last_bg_y = by;
    s->last_pal_count = pal;
    s->last_gfx_count = gfx;
    s->last_sprite_signature = sprites;
    s->stats = stats;
    ++s->draws;
    if (changed) *changed = true;
    return true;
}
