#ifndef DSI_TITLE_RUNNER_H
#define DSI_TITLE_RUNNER_H
/* Bounded scheduler for the UNMODIFIED TitleTask() while the rest of the
 * original task table is not linked. It does not implement any title rules. */
#include <stdint.h>
typedef enum {
    DSI_TITLE_RUNNING = 0,
    DSI_TITLE_AT_FILE_SELECT = 1,
    DSI_TITLE_UNEXPECTED_TASK = 2,
    DSI_TITLE_ASSET_FAILURE = 3,
    DSI_TITLE_TICK_LIMIT = 4
} DsiTitleRunResult;
typedef struct {
    uint32_t title_ticks;
    uint32_t entered_file_select_at;
    DsiTitleRunResult result;
} DsiTitleRunner;
void DsiTitleRunner_Init(DsiTitleRunner *runner);
DsiTitleRunResult DsiTitleRunner_Step(DsiTitleRunner *runner);
#endif
