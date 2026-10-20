#include "dsi_rom_refs.h"
#include "dsi_gba_bus.h"

bool DsiRomRefs_ReadU32(uint32_t gba_address, uint32_t *value) {
    uint8_t b[4];
    if (!value || (gba_address & 3u)) return false;
    if (!DsiGbaBus_Read(gba_address, b, sizeof b)) return false;
    *value = (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
             ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return true;
}

bool DsiRomRefs_ArrayEntry(uint32_t gba_array, uint32_t index, uint32_t *address) {
    if (!address || (gba_array & 3u) || index > (UINT32_MAX - gba_array) / 4u) return false;
    return DsiRomRefs_ReadU32(gba_array + index * 4u, address);
}

bool DsiRomRefs_Follow(uint32_t gba_pointer_location, void *out, size_t size) {
    uint32_t target;
    if ((!out && size) || !DsiRomRefs_ReadU32(gba_pointer_location, &target)) return false;
    /* No arbitrary host-memory dereferences; only ROM and shadow RAM. */
    if (target < 0x02000000u || (target >= 0x08000000u && target >= DSI_GBA_ROM_LIMIT)) return false;
    return DsiGbaBus_Read(target, out, size);
}
