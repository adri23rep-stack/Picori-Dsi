/* Host integration checks against UNMODIFIED original main.c/common.c and
 * the DSi shadow memory + SD-backed ROM cache adapter. No Nintendo SDK needed. */
#include "common.h"
#include "main.h"
#include "game.h"
#include "gba/io_reg.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include "dsi_engine_state.h"
#include "flags.h"
#include "area.h"
#include "save.h"
#include "room.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Input gInput;
Main gMain;

static void make_test_rom(const char *path) {
    FILE *f = fopen(path, "wb");
    assert(f);
    for (unsigned i = 0; i < 16384; ++i) {
        unsigned char b = (unsigned char)(((i * 101u) ^ (i >> 5)) & 255u);
        if (i == 0xAC) b = 'B';
        if (i == 0xAD) b = 'Z';
        if (i == 0xAE) b = 'M';
        if (i == 0xAF) b = 'P';
        assert(fwrite(&b, 1, 1, f) == 1);
    }
    fclose(f);
    f = fopen(path, "r+b"); assert(f);
    const uint8_t pointer[4] = { 0x08, 0x20, 0x00, 0x08 }; /* 0x08002008 */
    const uint8_t payload[6] = {0x42,0x13,0xAA,0x55,0x98,0xD1};
    assert(fseek(f, 0x1004, SEEK_SET) == 0);
    assert(fwrite(pointer, 1, sizeof(pointer), f) == sizeof(pointer));
    assert(fseek(f, 0x2008, SEEK_SET) == 0);
    assert(fwrite(payload, 1, sizeof(payload), f) == sizeof(payload));
    fclose(f);
}

int main(int argc, char **argv) {
    DsiGbaBus_Init();
    assert(DsiEngineState_SelfTest());
    /* Original game flags logic, including encoded global/local/room IDs */
    DsiEngineState_Reset();
    gArea.localFlagOffset = FLAG_BANK_4;
    SetFlag(0x4000u | 27u);
    SetFlag(0x8000u | 3u);
    SetFlag(0x0000u | 32u);
    assert(CheckFlags(0x4000u | 27u));
    assert(CheckFlags(0x8000u | 3u));
    assert(CheckFlags(0x0000u | 32u));
    ClearFlag(0x4000u | 27u);
    ClearFlag(0x8000u | 3u);
    ClearFlag(0x0000u | 32u);
    assert(!CheckFlags(0x4000u | 27u));
    assert(!CheckFlags(0x8000u | 3u));
    assert(!CheckFlags(0x0000u | 32u));
    assert(CheckGlobalFlags(10, 0) == TRUE);
    puts("PASS original flags.c: global/local/room flags and ranges");
    assert(gba_read16(0x04000130u) == 0x03FFu);
    /* Actual input logic, from unmodified src/common.c */
    ReadKeyInput();
    assert(gInput.heldKeys == 0);
    gba_write16(0x04000130u, 0x03FEu);
    ReadKeyInput();
    assert(gInput.heldKeys == 1 && gInput.newKeys == 1);
    ReadKeyInput();
    assert(gInput.heldKeys == 1 && gInput.newKeys == 0);
    gba_write16(0x04000130u, 0x03FFu);
    ReadKeyInput();
    assert(gInput.heldKeys == 0);
    puts("PASS original ReadKeyInput");

    /* Actual task state transitions from unmodified src/main.c */
    memset(&gMain, 0xFE, sizeof(gMain));
    SetTask(TASK_GAME);
    assert(gMain.task == TASK_GAME && gMain.state == GAMETASK_TRANSITION &&
           gMain.substate == GAMEMAIN_INITROOM);
    SetTask(TASK_TITLE);
    assert(gMain.task == TASK_TITLE && gMain.state == GAMETASK_TRANSITION &&
           gMain.substate == GAMEMAIN_INITROOM);
    puts("PASS original SetTask");

    gba_write8(0x02000041u, 0x72);
    assert(gba_read8(0x02000041u) == 0x72);
    gba_write16(0x03000060u, 0x1234u);
    assert(gba_read16(0x03000060u) == 0x1234u);
    gba_write32(0x06000020u, 0xCAFEBABEu);
    assert(gba_read32(0x06000020u) == 0xCAFEBABEu);
    gba_write16(0x05000202u, 0x2ABCu);
    assert(gba_read16(0x05000202u) == 0x2ABCu);
    gba_write32(0x050001FEu, 0x56781234u); /* crosses BG/OBJ palette seam */
    assert(gba_read32(0x050001FEu) == 0x56781234u);
    gba_write32(0x07000020u, 0xAABBCCDDu);
    assert(gba_read32(0x07000020u) == 0xAABBCCDDu);
    gba_write8(0x02000000u, 0xAAu);
    gba_write8(0x02000001u, 0xBBu);
    assert(DsiGbaBus_Copy(0x02000001u, 0x02000000u, 2));
    assert(gba_read8(0x02000001u) == 0xAAu);
    assert(gba_read8(0x02000002u) == 0xBBu);
    assert(!DsiGbaBus_Write(0x08000000u, "X", 1));
    assert(!DsiGbaBus_Read(0x01000000u, (char[1]){0}, 1));
    assert(!DsiGbaBus_Read(0x0203FFFFu, (char[2]){0}, 2));
    assert(DsiGbaBus_InvalidWrites() == 1 && DsiGbaBus_InvalidReads() == 2);
    puts("PASS GBA RAM/IO/VRAM/OAM, bounds, ROM write protection, memmove");

    char tempName[] = "/tmp/tmc-e2-test-XXXXXX";
    int tempFd = mkstemp(tempName);
    assert(tempFd >= 0);
    close(tempFd);
    make_test_rom(tempName);
    TmcRomReader rom = {0};
    assert(TmcRom_Open(&rom, tempName));
    DsiGbaBus_AttachRom(&rom);
    assert(rom.region == TMC_ROM_REGION_EUROPE);
    assert(gba_read32(0x080000ACu) == 0x504D5A42u); /* BZMP */
    uint8_t direct[60], mapped[60];
    assert(TmcRom_Read(&rom, 0xFF0u, direct, sizeof(direct)));
    assert(DsiGbaBus_Read(0x08000FF0u, mapped, sizeof(mapped)));
    assert(memcmp(direct, mapped, sizeof(mapped)) == 0);
    assert(DsiGbaBus_Copy(0x02000120u, 0x08000FF0u, sizeof(mapped)));
    uint32_t addr = 0;
    uint8_t data[6];
    assert(DsiRomRefs_ArrayEntry(0x08001004u, 0, &addr) && addr == 0x08002008u);
    assert(DsiRomRefs_Follow(0x08001004u, data, sizeof(data)));
    assert(memcmp(data, (uint8_t[]){0x42,0x13,0xAA,0x55,0x98,0xD1}, 6) == 0);
    assert(!DsiRomRefs_ArrayEntry(0x08001004u, UINT32_MAX, &addr));
    assert(!DsiRomRefs_Follow(0x08001004u + 1, data, sizeof(data)));
    puts("PASS streamed GBA ROM pointer table: follow/copy with bounds");
    assert(memcmp(&gEwram[0x120], direct, sizeof(direct)) == 0);
    assert(!DsiGbaBus_Read(0x08004000u, mapped, 1));
    assert(rom.hits > 0 && rom.misses >= 2);
    DsiGbaBus_DetachRom();
    TmcRom_Close(&rom);
    remove(tempName);
    puts("PASS paged ROM address bridge, crossing 4 KiB, direct ROM -> EWRAM copy");

    if (argc > 1) {
        TmcRomReader userRom = {0};
        assert(TmcRom_Open(&userRom, argv[1]));
        DsiGbaBus_AttachRom(&userRom);
        assert(gba_read32(0x080000ACu) == 0x504D5A42u);
        assert(DsiGbaBus_Read(0x08000FF0u, mapped, sizeof(mapped)));
        DsiGbaBus_DetachRom();
        TmcRom_Close(&userRom);
        puts("PASS provided EU ROM smoke test (no ROM redistributed)");
    }
    puts("PASS ENGINE E3 host integration. AgbMain NOT RUNNING.");
    return 0;
}
