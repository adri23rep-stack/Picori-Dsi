#ifndef DSI_E30_GATE_H
#define DSI_E30_GATE_H
#include <stdbool.h>
/* Guard, never a substitute game loop: no GameTask calls originate here. */
enum {
    DSI_E30_BLOCK_ROOM = 1u << 0,
    DSI_E30_BLOCK_STATE = 1u << 1,
    DSI_E30_BLOCK_PLAYER = 1u << 2,
    DSI_E30_BLOCK_LIST = 1u << 3,
    DSI_E30_BLOCK_HITBOX = 1u << 4,
    DSI_E30_BLOCK_INTERACTION = 1u << 5,
    DSI_E30_BLOCK_PROPERTIES = 1u << 6,
    DSI_E30_BLOCK_ENTITIES_INIT = 1u << 7,
    DSI_E30_BLOCK_SPRITE_LAYER = 1u << 8,
    DSI_E30_BLOCK_ENTITY_BANKS = 1u << 9
};
void DsiE30Gate_Reset(void);
/* Called ONLY after original static InitializeEntities in src/game.c returns. */
void DsiE30Gate_OriginalEntitiesCompleted(void);
unsigned DsiE30Gate_Blockers(void);
bool DsiE30Gate_Ready(void);
#endif
