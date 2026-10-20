#include "tmc_rom_stream.h"
#include <limits.h>
#include <string.h>

static TmcRomCacheEntry *GetPage(TmcRomReader *r, uint32_t page_index) {
    TmcRomCacheEntry *victim = NULL;
    uint32_t i;
    for (i = 0; i < TMC_ROM_CACHE_PAGES; ++i) {
        TmcRomCacheEntry *entry = &r->slots[i];
        if (entry->valid && entry->page_index == page_index) {
            ++r->hits;
            entry->last_used = ++r->clock;
            return entry;
        }
        if (!victim || !entry->valid ||
            (victim->valid && entry->last_used < victim->last_used)) {
            victim = entry;
        }
        if (victim && !victim->valid) break;
    }
    if (!victim) return NULL;

    ++r->misses;
    uint32_t start = page_index * TMC_ROM_PAGE_SIZE;
    if (start >= r->size) return NULL;
    size_t amount = r->size - start;
    if (amount > TMC_ROM_PAGE_SIZE) amount = TMC_ROM_PAGE_SIZE;
    if (fseek(r->file, (long)start, SEEK_SET) != 0) return NULL;
    if (fread(victim->data, 1, amount, r->file) != amount) return NULL;

    victim->page_index = page_index;
    victim->last_used = ++r->clock;
    victim->bytes_valid = (uint32_t)amount;
    victim->valid = true;
    return victim;
}

bool TmcRom_Read(TmcRomReader *r, uint32_t offset, void *dst, size_t length) {
    if (!r || !r->file || (!dst && length != 0)) return false;
    if ((uint64_t)offset + length > r->size) return false;
    uint8_t *out = (uint8_t *)dst;
    while (length != 0) {
        uint32_t in_page = offset % TMC_ROM_PAGE_SIZE;
        TmcRomCacheEntry *entry = GetPage(r, offset / TMC_ROM_PAGE_SIZE);
        if (!entry || in_page >= entry->bytes_valid) return false;
        size_t chunk = entry->bytes_valid - in_page;
        if (chunk > length) chunk = length;
        memcpy(out, entry->data + in_page, chunk);
        offset += (uint32_t)chunk;
        out += chunk;
        length -= chunk;
    }
    return true;
}

bool TmcRom_Open(TmcRomReader *r, const char *path) {
    if (!r || !path || r->file) return false;
    FILE *file = fopen(path, "rb");
    if (!file) return false;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return false; }
    long end = ftell(file);
    if (end < 0xB0 || (uint64_t)end > UINT32_MAX) { fclose(file); return false; }

    r->file = file;
    r->size = (uint32_t)end;
    char code[4];
    if (!TmcRom_Read(r, 0xAC, code, sizeof(code))) {
        TmcRom_Close(r);
        return false;
    }
    if (!memcmp(code, "BZME", sizeof(code))) r->region = TMC_ROM_REGION_USA;
    else if (!memcmp(code, "BZMP", sizeof(code))) r->region = TMC_ROM_REGION_EUROPE;
    else {
        TmcRom_Close(r);
        return false;
    }
    return true;
}

void TmcRom_Close(TmcRomReader *r) {
    if (!r) return;
    if (r->file) fclose(r->file);
    memset(r, 0, sizeof(*r));
}

TmcRegion TmcRom_GetRegion(const TmcRomReader *r) {
    return r ? r->region : TMC_ROM_REGION_UNSUPPORTED;
}
