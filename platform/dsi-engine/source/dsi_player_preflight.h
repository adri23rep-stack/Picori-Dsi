#ifndef DSI_E29_PLAYER_PREFLIGHT_H
#define DSI_E29_PLAYER_PREFLIGHT_H
/* Checks authentic original game state before enabling InitializePlayer.
 * Does NOT allocate, spawn or simulate any player/entity. */
#include <stdbool.h>
bool DsiPlayerPreflight_Ready(void);
#endif
