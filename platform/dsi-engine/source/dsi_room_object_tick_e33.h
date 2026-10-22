#ifndef DSI_ROOM_OBJECT_TICK_E33_H
#define DSI_ROOM_OBJECT_TICK_E33_H
#include <stdbool.h>
void DsiRoomObjectE33_Reset(void);
bool DsiRoomObjectE33_Tick(void);
bool DsiRoomObjectE33_Ready(void);
unsigned DsiRoomObjectE33_Ticks(void);
unsigned DsiRoomObjectE33_Errors(void);
/* E38: stable cause of blocked authentic bed tick, zero when successful. */
unsigned DsiRoomObjectE33_Reason(void);
#endif
