/* Game bootstrap boundary: deliberately NOT a replacement GameTask.
 * Use the authentic SaveFile layout, ResetSaveFile defaults from
 * src/fileselect.c, and SetTask from src/main.c. No persistent saves. */
#include "dsi_game_bootstrap.h"
#include "dsi_file_select_probe.h"
#include "fileselect.h"
#include "main.h"
#include "save.h"
#include "game.h"
#include <string.h>

DsiGameBootstrapStats gDsiGameBootstrapStats;

void DsiGameBootstrap_Reset(void) {
    memset(&gDsiGameBootstrapStats, 0, sizeof(gDsiGameBootstrapStats));
}

bool DsiGameBootstrap_Begin(unsigned slot) {
    ++gDsiGameBootstrapStats.attempts;
    /* Never recover automatically from the wrong task or use corrupt or
     * stale RAM slots. The caller must run DsiFileSelectProbe_Prepare(). */
    if (gMain.task != TASK_FILE_SELECT || slot >= NUM_SAVE_SLOTS ||
        gDsiFileSelectProbeStats.prepared_slots != NUM_SAVE_SLOTS ||
        gDsiFileSelectProbeStats.errors != 0 ||
        gFileSelectState.saveStatus[slot] != 0) {
        ++gDsiGameBootstrapStats.rejected;
        return false;
    }
    const SaveFile *source = &gFileSelectState.saves[slot];
    if (source->msg_speed != 1 || source->brightness != 1 ||
        source->stats.maxHealth != 24 || source->stats.health != 24) {
        ++gDsiGameBootstrapStats.rejected;
        return false;
    }
    /* RAM-only equivalent of the original menu selecting a save slot.
     * Never call SetActiveSave() here: its PC_PORT path can write files. */
    memcpy(&gSave, source, sizeof(gSave));
    gSaveHeader->saveFileId = (uint8_t)slot;
    gSaveHeader->msg_speed = gSave.msg_speed;
    gSaveHeader->brightness = gSave.brightness;
    /* AUTHENTIC SetTask implementation from src/main.c. This marks the
     * boundary to GameTask but does not execute a single gameplay tick. */
    SetTask(TASK_GAME);
    if (gMain.task != TASK_GAME || gMain.state != GAMETASK_TRANSITION ||
        gMain.substate != GAMEMAIN_INITROOM) {
        ++gDsiGameBootstrapStats.rejected;
        return false;
    }
    ++gDsiGameBootstrapStats.accepted;
    gDsiGameBootstrapStats.slot = (uint8_t)slot;
    gDsiGameBootstrapStats.area = gSave.saved_status.area_next;
    gDsiGameBootstrapStats.room = gSave.saved_status.room_next;
    gDsiGameBootstrapStats.ram_only = true;
    gDsiGameBootstrapStats.game_task_linked = false;
    return true;
}
