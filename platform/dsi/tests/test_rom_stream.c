#include "tmc_rom_stream.h"
#include "tmc_input.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void MakeFixture(const char *name, const char *code) {
    FILE *f = fopen(name, "wb");
    assert(f);
    for (int i = 0; i < 4096 * 12 + 13; ++i) {
        unsigned char c = (unsigned char)(i * 13 + 3);
        if (i >= 0xAC && i < 0xB0) c = (unsigned char)code[i - 0xAC];
        assert(fwrite(&c, 1, 1, f) == 1);
    }
    fclose(f);
}

static void CheckFixture(const char *path, const char *code, TmcRegion region) {
    MakeFixture(path, code);
    TmcRomReader reader = {0};
    assert(TmcRom_Open(&reader, path));
    assert(TmcRom_GetRegion(&reader) == region);
    assert(reader.size == 4096u * 12u + 13u);
    unsigned char buf[8199];
    assert(TmcRom_Read(&reader, 4094u, buf, sizeof(buf)));
    for (size_t i = 0; i < sizeof(buf); ++i) {
        assert(buf[i] == (unsigned char)((4094u + i) * 13u + 3u));
    }
    uint32_t before = reader.hits;
    assert(TmcRom_Read(&reader, 4094, buf, 2));
    assert(reader.hits > before);
    assert(!TmcRom_Read(&reader, reader.size - 2, buf, 4));
    assert(!TmcRom_Read(&reader, UINT32_MAX, buf, 8));
    assert(TmcRom_Read(&reader, reader.size, buf, 0));
    /* Trigger LRU eviction and reread page 0. */
    for (unsigned p = 0; p < 12; ++p) {
        assert(TmcRom_Read(&reader, p * 4096u, buf, 1));
    }
    assert(TmcRom_Read(&reader, 0, buf, 1));
    assert(buf[0] == 3);
    TmcRom_Close(&reader);
    assert(!reader.file);
    remove(path);
}

int main(void) {
    assert(TmcInput_MapNdsToGba(0) == 0x03FF);
    assert(TmcInput_MapNdsToGba(1u) == 0x03FE); /* A */
    assert(TmcInput_MapNdsToGba(1u << 6) == (0x03FFu & ~(1u << 6)));
    assert(TmcInput_MapNdsToGba(1u << 10) == 0x03FF); /* X ignored */
    CheckFixture("tmc_test_usa.gba", "BZME", TMC_ROM_REGION_USA);
    CheckFixture("tmc_test_eu.gba", "BZMP", TMC_ROM_REGION_EUROPE);
    MakeFixture("tmc_test_wrong.gba", "NOTZ");
    TmcRomReader unsupported = {0};
    assert(!TmcRom_Open(&unsupported, "tmc_test_wrong.gba"));
    assert(!unsupported.file);
    remove("tmc_test_wrong.gba");
    puts("PASS: 2 regions, page boundaries, cache, bounds, input mapping");
    return 0;
}
