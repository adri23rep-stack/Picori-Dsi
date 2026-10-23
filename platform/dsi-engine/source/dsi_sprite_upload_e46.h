#ifndef DSI_SPRITE_UPLOAD_E46_H
#define DSI_SPRITE_UPLOAD_E46_H
#include <stdint.h>
#include <stddef.h>
void DsiSpriteUploadE46_ValidateSlot(unsigned index);
void DsiSpriteUploadE46_Copy(uintptr_t source,uintptr_t destination,size_t bytes);
void DsiSpriteUploadE46_Lz(const void* source,void* destination);
#endif
