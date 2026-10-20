/* Storage for actual The Minish Cap engine state; NOT a replacement for
 * original gameplay code. This stage keeps it zero-initialized for tests. */
#include "area.h"
#include "room.h"
#include "save.h"
#include "flags.h"
#include "common.h"
#include "dsi_engine_state.h"
#include <string.h>

Area gArea;
RoomVars gRoomVars;
SaveFile gSave;

/* CheckBits is an assembly routine in the GBA original and currently has
 * a C implementation in the PC port. The equivalent portable implementation
 * is required by the UNMODIFIED src/flags.c. */
u32 CheckBits(void* bytes, u32 first, u32 count) {
    for (u32 i = 0; i < count; ++i) {
        if (!ReadBit(bytes, first + i)) return FALSE;
    }
    return TRUE;
}

/* The same helper is defined in src/playerUtils.c, which is not yet linked.
 * Keep this single adapter until that complete module can be ported. */
__attribute__((weak)) bool32 CheckLocalFlagByBank(u32 bank, u32 flag) {
    return ReadBit(gSave.flags, bank + flag);
}

void DsiEngineState_Reset(void) {
    memset(&gArea, 0, sizeof(gArea));
    memset(&gRoomVars, 0, sizeof(gRoomVars));
    memset(&gSave, 0, sizeof(gSave));
}

bool DsiEngineState_SelfTest(void) {
    DsiEngineState_Reset();
    /* Execute the REAL original flags.c implementations. Forbid persistence
     * here: these are isolated self-test data, never a user's saved game. */
    gArea.localFlagOffset = FLAG_BANK_3;
    SetGlobalFlag(17);
    if (!CheckGlobalFlag(17) || CheckGlobalFlag(18)) return false;
    SetLocalFlag(15);
    SetLocalFlag(16);
    if (!CheckLocalFlag(15) || !CheckLocalFlags(15, 2)) return false;
    ClearLocalFlag(16);
    if (CheckLocalFlags(15, 2) || !CheckGlobalFlag(17)) return false;
    SetRoomFlag(5);
    if (!CheckRoomFlag(5) || CheckRoomFlag(6)) return false;
    ClearRoomFlag(5);
    if (CheckRoomFlag(5)) return false;
    DsiEngineState_Reset();
    return true;
}
