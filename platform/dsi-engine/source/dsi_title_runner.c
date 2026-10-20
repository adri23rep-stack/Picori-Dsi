/* E13: link the actual title->file-select transition without executing an
 * unported FileSelectTask. No handwritten transition rules or fake UI. */
#include "dsi_title_runner.h"
#include "main.h"
#include "dsi_title_assets.h"
#include "dsi_title_entities.h"
#include "dsi_title_object_gfx.h"
#include "dsi_title_sprites.h"
#include <string.h>

#define TITLE_TICK_BUDGET 1500u
void DsiTitleRunner_Init(DsiTitleRunner *runner) {
    if (runner) memset(runner, 0, sizeof(*runner));
}
DsiTitleRunResult DsiTitleRunner_Step(DsiTitleRunner *runner) {
    if (!runner) return DSI_TITLE_UNEXPECTED_TASK;
    if (runner->result != DSI_TITLE_RUNNING) return runner->result;
    if (gMain.task == TASK_FILE_SELECT) {
        runner->entered_file_select_at = runner->title_ticks;
        return runner->result = DSI_TITLE_AT_FILE_SELECT;
    }
    if (gMain.task != TASK_TITLE) return runner->result = DSI_TITLE_UNEXPECTED_TASK;
    if (runner->title_ticks >= TITLE_TICK_BUDGET)
        return runner->result = DSI_TITLE_TICK_LIMIT;
    TitleTask(); /* unmodified ORIGINAL src/title.c */
    ++runner->title_ticks;
    if (gDsiTitleAssetStats.errors || gDsiTitleEntityStats.errors ||
        gDsiTitleObjectGfxStats.errors || gDsiTitleSpriteStats.errors)
        return runner->result = DSI_TITLE_ASSET_FAILURE;
    if (gMain.task == TASK_FILE_SELECT) {
        runner->entered_file_select_at = runner->title_ticks;
        return runner->result = DSI_TITLE_AT_FILE_SELECT;
    }
    if (gMain.task != TASK_TITLE) return runner->result = DSI_TITLE_UNEXPECTED_TASK;
    return DSI_TITLE_RUNNING;
}
