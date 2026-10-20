/* E6: ROM-backed title graphics service for ORIGINAL src/title.c.
 * Offsets from this project's kRomOffsets_EU in port/port_rom.c.
 * No game algorithms or title transitions are recreated here. */
#include "dsi_title_assets.h"
#include "dsi_gba_bus.h"
#include "dsi_rom_refs.h"
#include <string.h>
#include <limits.h>

#define EU_GFX_GROUP_TABLE 0x08100204u
#define EU_PALETTE_GROUP_TABLE 0x080FED88u
#define EU_GLOBAL_GFX_BASE 0x085A23D0u
#define EU_GFX_GROUP_COUNT 133u
#define EU_PALETTE_GROUP_COUNT 207u
#define EU_PALETTE_ID_MAX 0xffffu
#define MAX_GROUP_RECORDS 64u
#define MAX_LZ_OUTPUT 0x40000u

DsiTitleAssetStats gDsiTitleAssetStats;
static bool sEnabled;
static uint8_t sLanguage;

void DsiTitleAssets_Enable(bool enabled, uint8_t language) {
    sEnabled = enabled;
    sLanguage = language;
}
bool DsiTitleAssets_Active(void) { return sEnabled; }
void DsiTitleAssets_ResetStats(void) { memset(&gDsiTitleAssetStats, 0, sizeof(gDsiTitleAssetStats)); }
static bool fail(void) { ++gDsiTitleAssetStats.errors; return false; }
static bool read32(uint32_t address, uint32_t *value) { return DsiRomRefs_ReadU32(address, value); }
static bool romaddr(uint32_t address) {
    return address >= DSI_GBA_ROM_BASE && address < 0x09000000u;
}
static bool entry_ptr(uint32_t table, uint32_t group, uint32_t *pointer) {
    if (!read32(table + group * 4u, pointer) || !romaddr(*pointer) || (*pointer & 3u)) return false;
    return true;
}
static bool source_address(uint32_t offset, uint32_t *src) {
    if (offset > 0xFFFFFFu || offset > 0x09000000u - EU_GLOBAL_GFX_BASE) return false;
    *src = EU_GLOBAL_GFX_BASE + offset;
    return romaddr(*src);
}

/* Reads compressed bytes from ROM through the bounded cache. Never assumes
 * compressed data is adjacent to or permanently mapped in native memory. */
bool DsiTitleAssets_UnpackLz77(uint32_t gba_src, uint32_t gba_dst, uint32_t *out_len) {
    uint8_t header[4];
    if (!DsiGbaBus_Read(gba_src, header, 4) || header[0] != 0x10) return false;
    uint32_t length = (uint32_t)header[1] | ((uint32_t)header[2] << 8) | ((uint32_t)header[3] << 16);
    if (length == 0 || length > MAX_LZ_OUTPUT || !DsiGbaBus_IsWritable(gba_dst, length)) return false;
    uint8_t ring[4096] = {0};
    uint8_t staged[256];
    uint32_t cursor = gba_src + 4, out = 0, staged_count = 0, staged_dest = gba_dst;
    while (out < length) {
        uint8_t flags;
        if (!DsiGbaBus_Read(cursor++, &flags, 1)) return false;
        for (unsigned bit = 0; bit < 8 && out < length; ++bit) {
            uint32_t count = 1, distance = 0;
            uint8_t literal = 0;
            bool copy = (flags & (0x80u >> bit)) != 0;
            if (copy) {
                uint8_t b[2];
                if (!DsiGbaBus_Read(cursor, b, 2)) return false;
                cursor += 2;
                count = 3 + (b[0] >> 4);
                distance = 1 + (((uint32_t)b[0] & 0x0fu) << 8) + b[1];
                if (distance > out || count > length - out) return false;
            } else if (!DsiGbaBus_Read(cursor++, &literal, 1)) {
                return false;
            }
            for (uint32_t i = 0; i < count; ++i) {
                uint8_t value = copy ? ring[(out - distance) & 4095u] : literal;
                ring[out & 4095u] = value;
                staged[staged_count++] = value;
                ++out;
                if (staged_count == sizeof(staged)) {
                    if (!DsiGbaBus_Write(staged_dest, staged, staged_count)) return false;
                    staged_dest += staged_count;
                    staged_count = 0;
                }
            }
        }
    }
    if (staged_count && !DsiGbaBus_Write(staged_dest, staged, staged_count)) return false;
    if (out_len) *out_len = length;
    return true;
}

bool DsiTitleAssets_LoadGfxGroup(uint32_t group) {
    uint32_t ptr;
    if (!sEnabled || group >= EU_GFX_GROUP_COUNT ||
        !entry_ptr(EU_GFX_GROUP_TABLE, group, &ptr)) return fail();
    for (uint32_t index = 0; index < MAX_GROUP_RECORDS; ++index) {
        uint32_t record = ptr + index * 12u, cmd, dest, size;
        if (record < ptr || !read32(record, &cmd) || !read32(record + 4, &dest) ||
            !read32(record + 8, &size)) return fail();
        uint32_t ctrl = (cmd >> 24) & 0xfu;
        if (ctrl == 0xDu) {
            ++gDsiTitleAssetStats.gfx_groups;
            return true;
        }
        bool wanted = ctrl == 7 || (ctrl == 0xE && sLanguage != 0 && sLanguage != 1) ||
                      (ctrl == 0xF && sLanguage != 0) || ctrl == sLanguage;
        if (wanted) {
            uint32_t src, transferred = 0;
            if (!source_address(cmd & 0xFFFFFFu, &src)) return fail();
            if ((int32_t)size < 0) {
                if (!DsiTitleAssets_UnpackLz77(src, dest, &transferred)) return fail();
                ++gDsiTitleAssetStats.lz_blocks;
            } else {
                /* Original GBA DmaSet uses halfword count = size >> 1. */
                transferred = (size >> 1) * 2;
                if (!DsiGbaBus_Copy(dest, src, transferred)) return fail();
            }
            gDsiTitleAssetStats.bytes_gfx += transferred;
            ++gDsiTitleAssetStats.gfx_records;
        }
        if (!(cmd & 0x80000000u)) {
            ++gDsiTitleAssetStats.gfx_groups;
            return true;
        }
    }
    return fail();
}

bool DsiTitleAssets_LoadPaletteGroup(uint32_t group) {
    uint32_t ptr;
    if (!sEnabled || group >= EU_PALETTE_GROUP_COUNT ||
        !entry_ptr(EU_PALETTE_GROUP_TABLE, group, &ptr)) return fail();
    for (uint32_t index = 0; index < MAX_GROUP_RECORDS; ++index) {
        uint32_t packed, record = ptr + index * 4u;
        if (record < ptr || !read32(record, &packed)) return fail();
        uint32_t id = packed & EU_PALETTE_ID_MAX;
        uint32_t dest = (packed >> 16) & 0xffu;
        uint32_t count = (packed >> 24) & 0xFu;
        if (!count) count = 16;
        if (dest >= 32u || count > 32u - dest) return fail();
        uint32_t src;
        if (!source_address(id * 32u, &src)) return fail();
        if (!DsiGbaBus_Copy(0x05000000u + dest * 32u, src, count * 32u)) return fail();
        gDsiTitleAssetStats.bytes_pal += count * 32u;
        ++gDsiTitleAssetStats.palette_records;
        if (!(packed & 0x80000000u)) {
            ++gDsiTitleAssetStats.palette_groups;
            return true;
        }
    }
    return fail();
}
