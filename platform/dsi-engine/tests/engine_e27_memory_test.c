/* E27: exercise GBA ROM pointer format with synthetic bytes, not game logic. */
#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BASE 0x08000000u
static uint8_t image[256];
bool DsiGbaBus_Read(uint32_t address, void *dst, size_t size) {
    if (address < BASE || !dst) return false;
    uint32_t off = address - BASE;
    if (off > sizeof image || size > sizeof image - off) return false;
    memcpy(dst, image + off, size);
    return true;
}
static void write32(unsigned off, uint32_t value) {
    assert(off <= sizeof image - 4u);
    for (unsigned i=0; i<4; ++i) image[off+i] = (uint8_t)(value >> (i*8u));
}
int main(void) {
    uint32_t ptr = 0xDEADC0DEu;
    image[0x80] = 0x21; image[0x81] = 0x42; image[0x82] = 0x63;
    write32(0x20, BASE + 0x80u);
    assert(DsiRomRefs_ReadRomPointer(BASE + 0x20u, &ptr) && ptr == BASE + 0x80u);
    uint8_t bytes[3] = {0};
    assert(DsiRomRefs_Follow(BASE + 0x20u, bytes, sizeof bytes));
    assert(bytes[0] == 0x21 && bytes[1] == 0x42 && bytes[2] == 0x63);
    write32(0x40, BASE + 0x80u);
    write32(0x44, BASE + 0x90u);
    assert(DsiRomRefs_RomArrayEntry(BASE + 0x40u, 1u, &ptr) && ptr == BASE + 0x90u);
    assert(!DsiRomRefs_RomArrayEntry(BASE + 0x41u, 0, &ptr));
    assert(!DsiRomRefs_RomArrayEntry(BASE + 0x40u, UINT32_MAX, &ptr));
    assert(!DsiRomRefs_ReadRomPointer(BASE + 0x21u, &ptr));
    assert(!DsiRomRefs_ReadRomPointer(0x04000000u, &ptr));
    assert(!DsiRomRefs_IsRomSpan(0x08FFFFFFu, 2u));
    assert(DsiRomRefs_IsRomSpan(0x08FFFFFFu, 1u));
    assert(!DsiRomRefs_IsRomSpan(0x09000000u, 1u));
    assert(!DsiRomRefs_IsRomSpan(BASE, 0));
    /* Reject NULL, RAM, IO, region limits, addresses past actual ROM size. */
    const uint32_t bad[] = {0,0x02000000u,0x04000000u,
                            0x09000000u,BASE+0x300u,0xFFFFFFFFu};
    for(unsigned i=0;i<sizeof bad/sizeof bad[0];++i) {
        write32(0x30,bad[i]); ptr=0xACEDBEEFu;
        assert(!DsiRomRefs_ReadRomPointer(BASE+0x30u,&ptr));
        assert(ptr==0xACEDBEEFu);
    }
    puts("PASS E27: bounded GBA ROM pointer and table resolver");
    return 0;
}
