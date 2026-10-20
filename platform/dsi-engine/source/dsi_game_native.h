#ifndef DSI_GAME_NATIVE_H
#define DSI_GAME_NATIVE_H
#include <stdbool.h>
typedef struct {
    bool attempted;
    unsigned transition_ticks;
    unsigned init_ticks;
    unsigned room_prefix_ticks;
    unsigned deferred_services;
    unsigned errors;
} DsiGameNativeStats;
void DsiGameNative_Reset(void);
const DsiGameNativeStats *DsiGameNative_Status(void);
/* Dispatch ORIGINAL game.c only through transition + init, stop at MAIN. */
bool DsiGameNative_BootTwoStates(void);
/* Guard for the one-shot original GameMain_InitRoom prefix. */
bool DsiGameNative_AllowRoomTick(void);
bool DsiGameNative_BootRoomPrefix(void);
#endif
