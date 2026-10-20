#ifndef TMC_DSI_ROM_REFS_H
#define TMC_DSI_ROM_REFS_H
/* A temporary value-based ROM access API for source modules that use GBA
 * pointers stored inside the ROM. Never returns pointers to cache pages,
 * because those pointers become invalid after a cache eviction. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool DsiRomRefs_ReadU32(uint32_t gba_address, uint32_t *value);
bool DsiRomRefs_ArrayEntry(uint32_t gba_array, uint32_t index, uint32_t *address);
bool DsiRomRefs_Follow(uint32_t gba_pointer_location, void *out, size_t size);
#endif
