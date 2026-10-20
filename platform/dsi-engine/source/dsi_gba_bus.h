#ifndef DSI_GBA_BUS_H
#define DSI_GBA_BUS_H
/* Native DSi GBA-address bridge for the ORIGINAL Minish Cap engine.
 * GBA registers are SHADOWS here, never DS hardware addresses.
 * ROM reads are backed by TmcRomReader's bounded page cache on SD.
 * This deliberately cannot return stable pointers into ROM: porting callers
 * that directly dereference GBA ROM pointers is a separate migration step. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "tmc_rom_stream.h"

#define DSI_GBA_ROM_BASE 0x08000000u
#define DSI_GBA_ROM_LIMIT 0x0A000000u

/* Existing port_gba_mem.h API, implemented by this DSi backend. */
void gba_write8(uint32_t addr, uint8_t v);
void gba_write16(uint32_t addr, uint16_t v);
void gba_write32(uint32_t addr, uint32_t v);
uint8_t gba_read8(uint32_t addr);
uint16_t gba_read16(uint32_t addr);
uint32_t gba_read32(uint32_t addr);

void DsiGbaBus_Init(void);
void DsiGbaBus_AttachRom(TmcRomReader *reader);
void DsiGbaBus_DetachRom(void);
/* A failed read/write is signalled, not silently treated as valid ROM. */
bool DsiGbaBus_Read(uint32_t address, void *destination, size_t size);
bool DsiGbaBus_Write(uint32_t address, const void *source, size_t size);
bool DsiGbaBus_IsWritable(uint32_t address, size_t size);
bool DsiGbaBus_Copy(uint32_t destination, uint32_t source, size_t size);
unsigned DsiGbaBus_InvalidReads(void);
unsigned DsiGbaBus_InvalidWrites(void);
#endif
