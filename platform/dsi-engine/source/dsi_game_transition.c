/* E19: Validate the native game-state handoff without executing unported
 * services on a DSi. The original GameTask transition is tested on host. */
#include "dsi_game_transition.h"
#include "dsi_game_bootstrap.h"
#include "main.h"
#include "game.h"
#include "save.h"
#include <stdint.h>
#ifdef TMC_DSI_E31_LIFECYCLE
#include "dsi_room_property_data_e30.h"
#endif

bool DsiGameTransition_AllowNativeCopy(const void* src, u32 size) {
#ifdef TMC_DSI_E31_LIFECYCLE
    if (DsiRoomPropertyDataE30_AllowNativeCopy(src, size)) return true;
#endif
    uintptr_t p = (uintptr_t)src;
    uintptr_t start = (uintptr_t)&gSave.saved_status;
    if (p >= start && p - start <= sizeof(gSave.saved_status) &&
        size <= sizeof(gSave.saved_status) - (p - start)) return true;
    /* The ORIGINAL FinalizeSave() copies this immutable canonical filename
     * from ORIGINAL save.c when beginning a new game. Permit only its
     * exact range; never whitelist the rest of GBA-address ROM here. */
    if (src == (const void*)gUnk_0811E470 && size <= FILENAME_LENGTH - 1)
        return true;
    return false;
}

bool DsiGameTransition_Preflight(void) {
    return gMain.task == TASK_GAME &&
           gMain.state == GAMETASK_TRANSITION &&
           gMain.substate == GAMEMAIN_INITROOM &&
           gDsiGameBootstrapStats.ram_only &&
           gDsiGameBootstrapStats.accepted == 1 &&
           gSave.stats.maxHealth == 24 && gSave.stats.health == 24 &&
           DsiGameTransition_AllowNativeCopy(&gSave.saved_status,
                                              sizeof(gSave.saved_status));
}
