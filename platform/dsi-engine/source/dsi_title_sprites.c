/* Reads original EU GBA frame-OBJ structures, preserving the same 5-byte
 * OAM pieces and anchor table used by port/port_draw.c. Only hardware
 * transport and presentation are implemented here, not game sprite rules. */
#include "dsi_title_sprites.h"
#include "dsi_gba_bus.h"
#include <string.h>

#define EU_FRAME_LIST_BASE 0x082F3460u
#define EU_FRAME_LIST_SIZE 199561u
#define EU_ANCHOR_ROM 0x080B2310u
#define MAX_OAM_PIECES 128u
static DsiTitleSpritePiece pieces[MAX_OAM_PIECES];
static unsigned count;
static bool enabled, have_anchor;
static uint8_t anchors[240];
/* Four decoded ROM frame records: avoids extra SD page changes on every
 * frame when title.c draws a blinking prompt at 60 Hz. 4 * 641 B. */
typedef struct {
    uint16_t sprite;
    uint8_t frame, size, valid;
    uint8_t bytes[5*MAX_OAM_PIECES];
} CachedFrame;
static CachedFrame cache[4];
static unsigned next_cache;
DsiTitleSpriteStats gDsiTitleSpriteStats;

void DsiTitleSprites_Enable(bool value) {
    enabled = value;
    have_anchor = false;
    memset(cache, 0, sizeof(cache));
    next_cache = 0;
    memset(&gDsiTitleSpriteStats, 0, sizeof(gDsiTitleSpriteStats));
    DsiTitleSprites_Clear();
}
void DsiTitleSprites_Clear(void) { count = 0; }
unsigned DsiTitleSprites_Count(void) { return count; }
static bool fail(void) { ++gDsiTitleSpriteStats.errors; return false; }
static bool read_offset(uint32_t offset, void *dst, size_t n) {
    if (offset > EU_FRAME_LIST_SIZE || n > EU_FRAME_LIST_SIZE - offset) return false;
    return DsiGbaBus_Read(EU_FRAME_LIST_BASE + offset, dst, n);
}
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static bool prepare_anchor(void) {
    if (have_anchor) return true;
    if (!DsiGbaBus_Read(EU_ANCHOR_ROM, anchors, sizeof(anchors))) return false;
    /* Header of canonical four GBA square OBJ sizes. */
    static const uint8_t expected[16] = {
        0,0,8,8, 0,0,16,16, 0,0,32,32, 0,0,64,64
    };
    if (memcmp(anchors, expected, sizeof(expected))) return false;
    have_anchor = true;
    return true;
}

bool DsiTitleSprites_DrawDirect(uint32_t sprite, uint32_t frame, const OAMCommand *cmd) {
    if (!enabled || !cmd || frame == 0xFFu) return true;
    if (sprite >= 511u || frame >= 256u || !prepare_anchor()) return fail();
    const uint8_t *body = NULL;
    unsigned length = 0;
    for(unsigned c=0;c<4;c++) {
        if(cache[c].valid && cache[c].sprite==sprite && cache[c].frame==frame) {
            body=cache[c].bytes;
            length=cache[c].size;
            ++gDsiTitleSpriteStats.cache_hits;
            break;
        }
    }
    if(!body) {
        uint8_t raw[4];
        if(!read_offset(sprite*4u,raw,4)) return fail();
        uint32_t table=rd32(raw);
        if(table>EU_FRAME_LIST_SIZE || frame>(EU_FRAME_LIST_SIZE-table)/4u ||
           !read_offset(table+frame*4u,raw,4)) return fail();
        uint32_t location=rd32(raw);
        uint8_t n;
        if(!read_offset(location,&n,1) || n>MAX_OAM_PIECES ||
            (size_t)n*5u>EU_FRAME_LIST_SIZE-location-1u) return fail();
        CachedFrame *entry=&cache[next_cache++ % 4u];
        if(!read_offset(location+1u,entry->bytes,(size_t)n*5u)) return fail();
        entry->sprite=(uint16_t)sprite;
        entry->frame=(uint8_t)frame;
        entry->size=n;
        entry->valid=1;
        body=entry->bytes;
        length=n;
        ++gDsiTitleSpriteStats.cache_misses;
    }
    if(length>MAX_OAM_PIECES-count) return fail();
    uint32_t flags = (uint16_t)cmd->_4 | ((uint32_t)(uint16_t)cmd->_6 << 16);
    uint32_t off = (flags & 0x300u) ? ((flags & 0x300u) == 0x100u ? 0 : 0xC0u) :
        (((flags >> 24) & 0x30u) * 3u);
    if (off > sizeof(anchors) - 48u) return fail();
    /* Reject unsupported affine mode instead of interpreting the affine
     * matrix index as H/V flip. It is needed for the future sword zoom. */
    if (flags & 0x300u) return fail();
    ++gDsiTitleSpriteStats.draw_calls;
    unsigned before=count;
    for (unsigned i=0;i<length;i++) {
        const uint8_t *b = body+i*5u;
        unsigned info=b[2], size_off=off+((info & 0xF0u)>>2);
        if (size_off+3u>=sizeof(anchors)) return fail();
        int16_t xoff=(int8_t)b[0], yoff=(int8_t)b[1];
        if (flags & 0x10000000u) xoff=-xoff;
        if (flags & 0x20000000u) yoff=-yoff;
        int16_t x=(int16_t)(cmd->x + xoff - anchors[size_off]);
        int16_t y=(int16_t)(cmd->y + yoff - anchors[size_off+1u]);
        unsigned w=anchors[size_off+2u], h=anchors[size_off+3u];
        if (x>=240 || y>=160 || x+w<=0 || y+h<=0) continue;
        uint32_t packed=(uint32_t)(y & 255) | ((uint32_t)(x & 511)<<16) | flags |
            ((uint32_t)(info&0xC0u)<<8);
        packed ^= (uint32_t)(info&0x3Cu)<<26;
        uint16_t attr2=(uint16_t)(b[3]+cmd->_8);
        if (info & 1u) attr2 &= 0x0FFFu;
        attr2=(uint16_t)(attr2+((uint16_t)b[4]<<8));
        pieces[count].x=x; pieces[count].y=y;
        pieces[count].attr0=(uint16_t)packed;
        pieces[count].attr1=(uint16_t)(packed>>16);
        pieces[count].attr2=attr2;
        ++count;
    }
    gDsiTitleSpriteStats.pieces+=count-before;
    return true;
}

uint32_t DsiTitleSprites_Signature(void) {
    uint32_t hash=2166136261u;
    for (unsigned i=0;i<count;i++) {
        const DsiTitleSpritePiece *p=&pieces[i];
        const uint16_t words[]={ (uint16_t)p->x,(uint16_t)p->y,p->attr0,p->attr1,p->attr2 };
        for (unsigned j=0;j<5;j++) { hash^=words[j];hash*=16777619u; }
    }
    hash^=count;hash*=16777619u;
    return hash;
}

static bool dimensions(unsigned shape,unsigned size,unsigned *w,unsigned *h) {
    static const uint8_t sizes[3][4][2]={
        {{8,8},{16,16},{32,32},{64,64}},
        {{16,8},{32,8},{32,16},{64,32}},
        {{8,16},{8,32},{16,32},{32,64}}
    };
    if(shape>2 || size>3) return false;
    *w=sizes[shape][size][0];*h=sizes[shape][size][1];return true;
}

bool DsiTitleSprites_Composite(uint16_t *dst,size_t stride,
    const uint8_t *vram,size_t len,const uint16_t *pal,size_t pal_len,
    uint16_t dispcnt,unsigned *drawn_pixels) {
    if(drawn_pixels) *drawn_pixels=0;
    if(!dst || stride<256 || !vram || len<0x18000u || !pal || pal_len<256) return false;
    if(!(dispcnt & (1u<<12))) return true; /* original OBJ enable bit */
    unsigned painted=0;
    /* OAM entries with smaller index go on top: draw in reverse order. */
    for (unsigned oi=count;oi>0;--oi) {
        const DsiTitleSpritePiece *p=&pieces[oi-1];
        uint16_t a0=p->attr0,a1=p->attr1,a2=p->attr2;
        if((a0 & 0x0300u) || ((a0>>10)&3u)==2u) continue; /* affine/window unsupported */
        unsigned w,h;
        if(!dimensions((a0>>14)&3u,(a1>>14)&3u,&w,&h)) continue;
        bool eight=(a0&0x2000u)!=0, flipx=(a1&0x1000u)!=0,flipy=(a1&0x2000u)!=0;
        unsigned tile=a2&1023u,palette=(a2>>12)&15u;
        unsigned xstep=(dispcnt & (1u<<6)) ? (w/8u)*(eight?2u:1u) : 32u;
        for(unsigned y=0;y<h;y++) {
            int dy=p->y+(int)y;
            if(dy<0 || dy>=160) continue;
            unsigned sy=flipy?(h-1u-y):y;
            for(unsigned x=0;x<w;x++) {
                int dx=p->x+(int)x;
                if(dx<0 || dx>=240) continue;
                unsigned sx=flipx?(w-1u-x):x;
                size_t location=0x10000u+(size_t)(tile+(sy/8u)*xstep+(sx/8u)*(eight?2u:1u))*32u;
                location+=(sy&7u)*(eight?8u:4u)+(eight?(sx&7u):((sx&7u)>>1));
                if(location>=len) continue;
                unsigned index=vram[location];
                if(!eight) index=(index>>(4u*(sx&1u)))&15u;
                if(!index) continue;
                if(!eight) index+=palette*16u;
                dst[(dy+16)*stride+dx+8]=(uint16_t)(0x8000u|(pal[index]&0x7FFFu));
                ++painted;
            }
        }
    }
    gDsiTitleSpriteStats.rendered_pixels+=painted;
    if(drawn_pixels) *drawn_pixels=painted;
    return true;
}

/* E34 presentation-only palette remap for a SINGLE original ROM sprite.
 * This changes synthesized OAM attributes, never the source frame pieces. */
bool DsiTitleSprites_UsePaletteBank(unsigned bank) {
    if (bank>=16u || !count) return false;
    for(unsigned i=0;i<count;i++)
        if (pieces[i].attr0 & 0x2000u) return false; /* 8bpp is not banked */
    for(unsigned i=0;i<count;i++)
        pieces[i].attr2=(uint16_t)((pieces[i].attr2 & 0x0fffu) | (bank<<12));
    return true;
}
