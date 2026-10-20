/* Test cross-segment RAM copy using exactly the DSi shadow-memory backend.
 * Includes genuine read-only ROM -> EWRAM streaming with cache page crossing. */
#include "dsi_gba_bus.h"
#include "port_gba_mem.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void must_read(uint32_t a, void *p, size_t n) {
    assert(DsiGbaBus_Read(a, p, n));
}
static void must_write(uint32_t a, const void *p, size_t n) {
    assert(DsiGbaBus_Write(a, p, n));
}
int main(int argc, char **argv) {
    assert(argc == 2);
    DsiGbaBus_Init();
    TmcRomReader rom = {0};
    assert(TmcRom_Open(&rom, argv[1]));
    assert(rom.region == TMC_ROM_REGION_EUROPE);
    DsiGbaBus_AttachRom(&rom);
    unsigned char src[320], expected[320], actual[320];
    for (size_t i=0;i<sizeof(src);i++) src[i] = (unsigned char)(i * 17u + 31u);

    /* Cross the split between shadow BG and OBJ palette arrays. */
    must_write(0x05000180, src, sizeof(src));
    must_read(0x05000180, expected, sizeof(expected));
    assert(!memcmp(expected,src,sizeof(src)));
    assert(DsiGbaBus_Copy(0x050001C0, 0x05000180, 256));
    memmove(expected+0x40, expected, 256);
    must_read(0x05000180, actual, sizeof(actual));
    assert(!memcmp(expected,actual,sizeof(expected)));

    /* Opposite overlap direction, still crossing both palette banks. */
    must_write(0x05000180, src, sizeof(src));
    memcpy(expected,src,sizeof(src));
    assert(DsiGbaBus_Copy(0x05000180,0x050001C0,256));
    memmove(expected,expected+0x40,256);
    must_read(0x05000180,actual,sizeof(actual));
    assert(!memcmp(expected,actual,sizeof(expected)));

    /* RAM copy to another segment; memcpy-style copy with distinct ranges. */
    must_write(0x05000180, src, sizeof(src));
    assert(DsiGbaBus_Copy(0x02001000, 0x05000180, sizeof(src)));
    must_read(0x02001000,actual,sizeof(actual));
    assert(!memcmp(actual,src,sizeof(src)));

    /* Source crossing an unmapped region must fail BEFORE touching target. */
    memset(expected,0xA5,sizeof(expected));
    must_write(0x02001000,expected,sizeof(expected));
    unsigned before_fail = DsiGbaBus_InvalidReads();
    assert(!DsiGbaBus_Copy(0x02001000,0x03007FF0,64));
    assert(DsiGbaBus_InvalidReads() == before_fail + 1);
    must_read(0x02001000,actual,sizeof(actual));
    assert(!memcmp(actual,expected,sizeof(expected)));

    /* True ROM streaming, including a 4 KiB SD cache page boundary. */
    unsigned char rombytes[64];
    assert(TmcRom_Read(&rom,0xFF0,rombytes,sizeof(rombytes)));
    assert(DsiGbaBus_Copy(0x03000100,0x08000FF0,sizeof(rombytes)));
    must_read(0x03000100,actual,sizeof(rombytes));
    assert(!memcmp(rombytes,actual,sizeof(rombytes)));
    assert(DsiGbaBus_Copy(0x02001000,0x02001000,sizeof(src)));
    assert(DsiGbaBus_InvalidWrites() == 0);
    puts("PASS E16: cross-palette RAM moves, both overlap directions, preflight, ROM cache-page crossing");
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    return 0;
}
