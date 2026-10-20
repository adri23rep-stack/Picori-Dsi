#ifndef DSI_GAME_BOOTSTRAP_H
#define DSI_GAME_BOOTSTRAP_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    unsigned attempts;
    unsigned accepted;
    unsigned rejected;
    uint8_t slot;
    uint8_t area;
    uint8_t room;
    bool ram_only;
    bool game_task_linked;
} DsiGameBootstrapStats;
extern DsiGameBootstrapStats gDsiGameBootstrapStats;
/* E18: prepare the genuine SaveFile in RAM and hand the ORIGINAL main task
 * selector to TASK_GAME. This deliberately DOES NOT call GameTask(): the
 * game/room/entity/audio dependencies are not linked yet. No disk I/O. */
bool DsiGameBootstrap_Begin(unsigned slot);
void DsiGameBootstrap_Reset(void);
#endif
