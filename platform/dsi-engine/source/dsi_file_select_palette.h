#ifndef DSI_FILE_SELECT_PALETTE_H
#define DSI_FILE_SELECT_PALETTE_H
#include <stdbool.h>
extern unsigned gDsiFileSelectPaletteTicks;
void DsiFileSelectPalette_Reset(void);
bool DsiFileSelectPalette_Initialize(void);
bool DsiFileSelectPalette_Step(void);
bool DsiFileSelectPalette_Ready(void);
#endif
