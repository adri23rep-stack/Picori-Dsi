/* Native storage for the ORIGINAL engine's GBA linker-owned globals. */
#include "entity.h"
#include "script.h"
#include "color.h"
#include "port_entity_ctx.h"
UpdateContext gUpdateContext;
CarriedEntity gCarriedEntity;
Palette gPaletteList[16];
struct_gUnk_020000C0 gUnk_020000C0[48];
u8 gzHeap[0x1000] __attribute__((aligned(4)));
ActiveScriptInfo gActiveScriptInfo;
ScriptExecutionContext gScriptExecutionContextArray[32];
ScriptExecutionContext gPlayerScriptExecutionContext;
ScriptExecutionContext* gEntityScriptCtxTable[PC_MAX_ENTITY_SLOTS];
/* Desktop logging hooks: telemetry only, never mutate gameplay state. */
volatile unsigned gDsiLifecycleEvents;
void Port_LogEntityEvent(const char* op,void* e,unsigned k,unsigned id,void* p,void* n) {
    (void)op;(void)e;(void)k;(void)id;(void)p;(void)n; ++gDsiLifecycleEvents;
}
void Port_DumpOrchStack(const char* tag,void* e) {(void)tag;(void)e; ++gDsiLifecycleEvents;}
void Port_LogOrchEvent(const char* op,void* e) {(void)op;(void)e; ++gDsiLifecycleEvents;}
