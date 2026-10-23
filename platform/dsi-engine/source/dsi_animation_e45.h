#ifndef DSI_ANIMATION_E45_H
#define DSI_ANIMATION_E45_H
#include <stdbool.h>
#include <stddef.h>
#include "entity.h"
#include "structures.h"
/* ROM transport supports original Link sprites 1..9 and 12.
 * Does not choose player actions/animations or upload/render graphics. */
bool DsiAnimationE45_Prepare(void);
bool DsiAnimationE45_Ready(void);
bool DsiAnimationE45_IsTileSpan(const void* pointer,size_t bytes);
bool DsiAnimationE45_CanInitialize(u16 sprite,u32 animation);
const SpritePtr* DsiAnimationE45_Sprite(u16 sprite);
const u8* DsiAnimationE45_Data(u16 sprite,u32 animation);
SpriteFrame* DsiAnimationE45_Frame(u16 sprite,u8 frame);
int DsiAnimationE45_RequireBytes(const void* ptr,size_t count);
u8* DsiAnimationE45_LoopTarget(u8* next,u8 frames);
#endif
