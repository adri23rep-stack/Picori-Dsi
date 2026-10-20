/* GBA memory shadow + SD ROM page bridge for native Nintendo DSi.
 * No Zelda gameplay algorithms are implemented here. */
#include "dsi_gba_bus.h"
#include <string.h>
#include "port_gba_mem.h"

/* GBA RAM and video state (not DS physical memory). Total ~416 KiB. */
u8 gIoMem[0x400] __attribute__((aligned(4)));
u8 gEwram[0x40000] __attribute__((aligned(4)));
u8 gIwram[0x8000] __attribute__((aligned(4)));
u16 gBgPltt[256] __attribute__((aligned(4)));
u16 gObjPltt[256] __attribute__((aligned(4)));
u16 gOamMem[0x400 / 2] __attribute__((aligned(4)));
u8 gVram[0x18000] __attribute__((aligned(4)));

static TmcRomReader *sRom;
static unsigned sInvalidReadCount, sInvalidWriteCount;

typedef struct { uint32_t base; size_t length; uint8_t *bytes; } Segment;

/* Return a contiguous segment available at this address. Distinct palette
 * arrays are never assumed adjacent in native memory. */
static bool ram_segment(uint32_t addr, uint8_t **out, size_t *available) {
    const Segment seg[] = {
        {0x02000000u, sizeof(gEwram), gEwram},
        {0x03000000u, sizeof(gIwram), gIwram},
        {0x04000000u, sizeof(gIoMem), gIoMem},
        {0x05000000u, sizeof(gBgPltt), (uint8_t*)gBgPltt},
        {0x05000200u, sizeof(gObjPltt), (uint8_t*)gObjPltt},
        {0x06000000u, sizeof(gVram), gVram},
        {0x07000000u, sizeof(gOamMem), (uint8_t*)gOamMem},
    };
    for (size_t i=0; i<sizeof(seg)/sizeof(seg[0]); ++i) {
        if (addr < seg[i].base) continue;
        uint32_t off = addr - seg[i].base;
        if (off < seg[i].length) {
            *out = seg[i].bytes + off;
            *available = seg[i].length - off;
            return true;
        }
    }
    return false;
}
static bool ram_range(uint32_t addr, size_t size) {
    while (size) {
        uint8_t *bytes;
        size_t amount;
        if (!ram_segment(addr, &bytes, &amount)) return false;
        if (amount > size) amount = size;
        if (amount > UINT32_MAX - addr) return false;
        addr += (uint32_t)amount;
        size -= amount;
    }
    return true;
}
static void ram_transfer(uint32_t addr, void *buffer, size_t size, bool write) {
    uint8_t *p = (uint8_t*)buffer;
    while (size) {
        uint8_t *bytes;
        size_t amount;
        /* Caller prevalidates full range. */
        (void)ram_segment(addr, &bytes, &amount);
        if (amount > size) amount = size;
        if (write) memcpy(bytes, p, amount);
        else memcpy(p, bytes, amount);
        addr += (uint32_t)amount;
        p += amount;
        size -= amount;
    }
}
void DsiGbaBus_Init(void) {
    memset(gIoMem, 0, sizeof(gIoMem));
    memset(gEwram, 0, sizeof(gEwram));
    memset(gIwram, 0, sizeof(gIwram));
    memset(gBgPltt, 0, sizeof(gBgPltt));
    memset(gObjPltt, 0, sizeof(gObjPltt));
    memset(gOamMem, 0, sizeof(gOamMem));
    memset(gVram, 0, sizeof(gVram));
    sRom = NULL;
    sInvalidReadCount = sInvalidWriteCount = 0;
    /* GBA buttons are active low: no button pressed. */
    gIoMem[0x130] = 0xFF;
    gIoMem[0x131] = 0x03;
}
void DsiGbaBus_AttachRom(TmcRomReader *reader) { sRom = reader && reader->file ? reader : NULL; }
void DsiGbaBus_DetachRom(void) { sRom = NULL; }

bool DsiGbaBus_Read(uint32_t address, void *destination, size_t size) {
    if (size == 0) return true;
    if (!destination) { ++sInvalidReadCount; return false; }
    if (ram_range(address, size)) {
        ram_transfer(address, destination, size, false);
        return true;
    }
    if (sRom && address >= DSI_GBA_ROM_BASE && address < DSI_GBA_ROM_LIMIT &&
        size <= (size_t)(DSI_GBA_ROM_LIMIT - address) &&
        TmcRom_Read(sRom, address - DSI_GBA_ROM_BASE, destination, size)) return true;
    ++sInvalidReadCount;
    return false;
}
bool DsiGbaBus_IsWritable(uint32_t address, size_t size) { return ram_range(address, size); }
bool DsiGbaBus_Write(uint32_t address, const void *source, size_t size) {
    if (size == 0) return true;
    if (!source || !ram_range(address, size)) {
        ++sInvalidWriteCount;
        return false;
    }
    ram_transfer(address, (void*)source, size, true);
    return true;
}

/* A bulk path for original DMA/MemCopy adaptation; constant RAM ranges or ROM
 * to RAM. Overlap in a single backing region uses real memmove semantics. */
bool DsiGbaBus_Copy(uint32_t destination, uint32_t source, size_t size) {
    uint8_t *dst = NULL, *src = NULL;
    size_t dstLeft = 0, srcLeft = 0;
    if (size == 0) return true;
    if (!ram_range(destination, size)) { ++sInvalidWriteCount; return false; }
    /* A single contiguous segment can use memmove directly. */
    if (ram_segment(destination, &dst, &dstLeft) && dstLeft >= size &&
        ram_segment(source, &src, &srcLeft) && srcLeft >= size) {
        memmove(dst, src, size);
        return true;
    }
    /* The GBA palette banks are two separate host arrays even though the
     * emulated palette address range is continuous. Original engine code
     * can legitimately transfer data across these boundaries. Validate
     * BOTH full ranges before copying, so no partial write can be made
     * on an invalid RAM source or destination. Reverse traversal preserves
     * memmove semantics for overlapping numeric GBA addresses. */
    if (ram_range(source, size)) {
        uint8_t scratch[256];
        if (destination > source && (uint64_t)destination - source < size) {
            size_t remaining = size;
            while (remaining) {
                size_t n = remaining < sizeof(scratch) ? remaining : sizeof(scratch);
                size_t offset = remaining - n;
                if (!DsiGbaBus_Read(source + (uint32_t)offset, scratch, n) ||
                    !DsiGbaBus_Write(destination + (uint32_t)offset, scratch, n)) return false;
                remaining -= n;
            }
        } else {
            for (size_t offset = 0; offset < size;) {
                size_t n = size - offset;
                if (n > sizeof(scratch)) n = sizeof(scratch);
                if (!DsiGbaBus_Read(source + (uint32_t)offset, scratch, n) ||
                    !DsiGbaBus_Write(destination + (uint32_t)offset, scratch, n)) return false;
                offset += n;
            }
        }
        return true;
    }
    /* Only a ROM -> RAM copy is otherwise supported here; refuse unknown
     * sources instead of copying partial/corrupted game data. */
    if (!sRom || source < DSI_GBA_ROM_BASE || source >= DSI_GBA_ROM_LIMIT ||
        size > (size_t)(DSI_GBA_ROM_LIMIT - source) ||
        size > (size_t)sRom->size ||
        source - DSI_GBA_ROM_BASE > sRom->size - size) {
        ++sInvalidReadCount;
        return false;
    }
    uint8_t temporary[512];
    while (size) {
        size_t step = size < sizeof(temporary) ? size : sizeof(temporary);
        if (!DsiGbaBus_Read(source, temporary, step) ||
            !DsiGbaBus_Write(destination, temporary, step)) return false;
        destination += (uint32_t)step;
        source += (uint32_t)step;
        size -= step;
    }
    return true;
}
unsigned DsiGbaBus_InvalidReads(void) { return sInvalidReadCount; }
unsigned DsiGbaBus_InvalidWrites(void) { return sInvalidWriteCount; }

/* Same exported function names as existing PC backend port_gba_mem.c.
 * These give original engine modules a source-compatible interface. */
uint8_t gba_read8(uint32_t a) { uint8_t v = 0; (void)DsiGbaBus_Read(a, &v, 1); return v; }
uint16_t gba_read16(uint32_t a) {
    uint8_t b[2] = {0}; (void)DsiGbaBus_Read(a, b, 2);
    return (uint16_t)(b[0] | ((uint16_t)b[1] << 8));
}
uint32_t gba_read32(uint32_t a) {
    uint8_t b[4] = {0}; (void)DsiGbaBus_Read(a, b, 4);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
void gba_write8(uint32_t a, uint8_t v) { (void)DsiGbaBus_Write(a, &v, 1); }
void gba_write16(uint32_t a, uint16_t v) {
    uint8_t b[2] = {(uint8_t)v, (uint8_t)(v >> 8)};
    (void)DsiGbaBus_Write(a, b, 2);
}
void gba_write32(uint32_t a, uint32_t v) {
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)};
    (void)DsiGbaBus_Write(a, b, 4);
}
