#ifndef TMC_ROM_STREAM_H
#define TMC_ROM_STREAM_H

/* Portable ROM reader.  Small page cache; does not retain the 16 MiB ROM. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define TMC_ROM_PAGE_SIZE 4096u
#define TMC_ROM_CACHE_PAGES 8u

typedef enum TmcRegion {
    TMC_ROM_REGION_UNSUPPORTED = 0,
    TMC_ROM_REGION_USA,
    TMC_ROM_REGION_EUROPE
} TmcRegion;

typedef struct TmcRomCacheEntry {
    uint32_t page_index;
    uint32_t last_used;
    uint32_t bytes_valid;
    bool valid;
    uint8_t data[TMC_ROM_PAGE_SIZE];
} TmcRomCacheEntry;

typedef struct TmcRomReader {
    FILE *file;
    uint32_t size;
    uint32_t clock;
    uint32_t hits;
    uint32_t misses;
    TmcRegion region;
    TmcRomCacheEntry slots[TMC_ROM_CACHE_PAGES];
} TmcRomReader;

/* The caller must first initialize the struct to zero (e.g. static or = {0}).
 * Open rejects unknown game codes, but does NOT authenticate the game's SHA-1.
 * The caller owns the ROM; no assets are bundled with the program. */
bool TmcRom_Open(TmcRomReader *reader, const char *path);
void TmcRom_Close(TmcRomReader *reader);
bool TmcRom_Read(TmcRomReader *reader, uint32_t offset, void *dst, size_t length);
TmcRegion TmcRom_GetRegion(const TmcRomReader *reader);

#endif
