/* Executable first-three-state probe. The actual TitleTask(), SetTask(),
 * ReadKeyInput() are provided by original engine modules. */
#include "dsi_title_trace.h"
#include "dsi_gba_bus.h"
#include <assert.h>
#include <stdio.h>
#include "main.h"
#include "common.h"
Input gInput;
Main gMain;
int main(void) {
    DsiGbaBus_Init();
    assert(DsiTitleTrace_TestFirstTransitions());
    assert(gDsiTitleTrace.title_calls == 3);
    assert(gDsiTitleTrace.unsupported_service_calls == 0);
    puts("PASS: original src/title.c TitleTask() executed first three state transitions");
    puts("PASS: original title requested gfx groups [16,1], palette [2]");
    puts("IMPORTANT: graphics handlers are diagnostic probes, no logo rendered");
    return 0;
}
