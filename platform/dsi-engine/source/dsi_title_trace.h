#ifndef DSI_TITLE_TRACE_H
#define DSI_TITLE_TRACE_H
#include <stdbool.h>
#include <stdint.h>
/* First two transitions of UNMODIFIED src/title.c. The service adapters
 * are instrumented stand-ins, NOT a GBA renderer or a playable title. */
typedef struct {
    unsigned title_calls;
    unsigned gfx_count;
    unsigned gfx_group[4];
    unsigned palette_count;
    unsigned palette_group[4];
    unsigned fade_count;
    unsigned message_count;
    unsigned disp_reset_count;
    unsigned oam_count;
    unsigned unsupported_service_calls;
    unsigned state_after_init;
    unsigned state_after_logo;
} DsiTitleTrace;
extern DsiTitleTrace gDsiTitleTrace;
void DsiTitleTrace_Reset(void);
bool DsiTitleTrace_TestFirstTransitions(void);
#endif
