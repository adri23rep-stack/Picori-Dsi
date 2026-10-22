/* E7 original TitleTask integration. Services still needing the full engine
 * are diagnostic hooks; LoadGfxGroup/LoadPaletteGroup load real EU ROM bytes.
 * ResetScreenRegs() is the unmodified original implementation; the E7
 * renderer consumes its BG state rather than inventing title coordinates. */
#include "main.h"
#include "common.h"
#include "game.h"
#include "save.h"
#include "fade.h"
#include "screen.h"
#include "entity.h"
#include "sound.h"
#include "gba/syscall.h"
#include "port_tts.h"
#include "dsi_title_trace.h"
#include "dsi_title_assets.h"
#include "dsi_title_sprites.h"
#include "dsi_title_entities.h"
#include <string.h>
#include <stdint.h>

DsiTitleTrace gDsiTitleTrace;
GfxSlotList gGFXSlots;
Screen gScreen;
OAMCommand gOamCmd;
FadeControl gFadeControl;
struct_02000010 gUnk_02000010;
UI gUI;
/* Original src/common.c exports ResetScreenRegs(); it uses the existing
 * gEwram-backed BG buffer macros from the original engine headers. */
void ResetScreenRegs(void);
/* PC_PORT's src/common.c MemClear calls this in the same translation unit,
 * so ld --wrap=MemClear cannot intercept it. For E7's three title ticks all
 * these requests target genuine C pointers (gScreen, intro/UI buffers).
 * Do not use this narrow shim as the eventual complete port memory bridge. */
void *port_resolve_write_addr(uintptr_t address) { return (void *)address; }
u8 _gMenuSharedStorage[0x40] __attribute__((aligned(8)));

void DsiTitleTrace_Reset(void) {
    DsiTitleSprites_Clear();
    DsiTitleEntities_Clear();
    memset(&gDsiTitleTrace, 0, sizeof(gDsiTitleTrace));
    memset(&gGFXSlots, 0, sizeof(gGFXSlots));
    memset(&gScreen, 0, sizeof(gScreen));
    memset(&gFadeControl, 0, sizeof(gFadeControl));
    memset(&gUI, 0, sizeof(gUI));
    memset(&gUnk_02000010, 0, sizeof(gUnk_02000010));
    memset(_gMenuSharedStorage, 0, sizeof(_gMenuSharedStorage));
    memset(&gMain, 0, sizeof(gMain));
}

bool DsiTitleTrace_TestFirstTransitions(void) {
    DsiTitleTrace_Reset();
    /* EU title graphics language slots begin at 2 (English).
     * This uses the existing save-header mapping, not a copy of it. */
    gSaveHeader->language = RegionDefaultLanguage();
    SetTask(TASK_TITLE); /* ORIGINAL main.c */
    TitleTask();          /* ORIGINAL title.c: transition to logo sequence */
    ++gDsiTitleTrace.title_calls;
    gDsiTitleTrace.state_after_init = gMain.state;
    if (gMain.state != GAMETASK_MAIN || gDsiTitleTrace.message_count != 1 ||
        gDsiTitleTrace.gfx_count != 0) return false;
    /* The original task starts in state=2 after its first call, then
     * transitions through state=1. Real fades would delay this. Our
     * diagnostic hooks deliberately leave fading inactive. */
    TitleTask();          /* ORIGINAL title.c: init -> logo dispatcher */
    ++gDsiTitleTrace.title_calls;
    if (gMain.state != GAMETASK_INIT) return false;
    TitleTask();          /* ORIGINAL title.c: requests first logo assets */
    ++gDsiTitleTrace.title_calls;
    gDsiTitleTrace.state_after_logo = gMain.state;
    return gMain.state == GAMETASK_INIT &&
           gDsiTitleTrace.gfx_count == 2 &&
           gDsiTitleTrace.gfx_group[0] == 16 &&
           gDsiTitleTrace.gfx_group[1] == 1 &&
           gDsiTitleTrace.palette_count == 1 &&
           gDsiTitleTrace.palette_group[0] == 2 &&
           gDsiTitleTrace.fade_count == 2 &&
           gDsiTitleTrace.oam_count == 3 &&
           gDsiTitleTrace.disp_reset_count == 2 &&
           gDsiTitleTrace.unsupported_service_calls == 0;
}

/* E5-only boundary probes. Mark unsupported accesses; they must be replaced
 * before enabling gameplay or claiming the real title screen has rendered. */
void __wrap_MemClear(void* dest, u32 len) { memset(dest, 0, len); }
void FlushSprites(void) { DsiTitleSprites_Clear(); }
void CopyOAM(void) { ++gDsiTitleTrace.oam_count; }
#ifdef TMC_DSI_E26_ORIGINAL_MESSAGE
/* Trace the call, then execute the UNMODIFIED original function. */
void __real_MessageInitialize(void);
void __wrap_MessageInitialize(void) {
    ++gDsiTitleTrace.message_count;
    __real_MessageInitialize();
}
#else
void MessageInitialize(void) { ++gDsiTitleTrace.message_count; }
#endif
/* Execute the ORIGINAL ResetScreenRegs() from src/common.c so BG2 control
 * and screen base come from game code (0x1D02), not a hardcoded E7 scene.
 * DispReset itself needs further hardware/other modules and stays wrapped. */
void __wrap_DispReset(bool32 refresh) {
    ++gDsiTitleTrace.disp_reset_count;
    ResetScreenRegs();
    gScreen.bg0.updated = (u16)refresh;
}
#ifdef TMC_DSI_E28_ORIGINAL_INITROOM
/* Keep the proven TITLE diagnostic behavior (no active fade) while
 * forwarding real GAME room transitions to original src/fade.c. */
void __real_SetFade(u32 type, u32 speed);
void __wrap_SetFade(u32 type, u32 speed) {
    ++gDsiTitleTrace.fade_count;
    if (gMain.task == TASK_GAME) __real_SetFade(type, speed);
}
#else
void SetFade(u32 type, u32 speed) { (void)type; (void)speed; ++gDsiTitleTrace.fade_count; }
#endif
void __wrap_LoadGfxGroup(u32 group) {
    unsigned idx = gDsiTitleTrace.gfx_count++;
    if (idx < 4) gDsiTitleTrace.gfx_group[idx] = group;
    if (DsiTitleAssets_Active()) (void)DsiTitleAssets_LoadGfxGroup(group);
}
void __wrap_LoadPaletteGroup(u32 group) {
    unsigned idx = gDsiTitleTrace.palette_count++;
    if (idx < 4) gDsiTitleTrace.palette_group[idx] = group;
    if (DsiTitleAssets_Active()) (void)DsiTitleAssets_LoadPaletteGroup(group);
}
void ResetPaletteTable(u32 a1) { (void)a1; ++gDsiTitleTrace.unsupported_service_calls; }
void ResetPalettes(void) { ++gDsiTitleTrace.unsupported_service_calls; }
#ifdef TMC_DSI_E31_LIFECYCLE
void __real_EraseAllEntities(void);
void __wrap_EraseAllEntities(void) {
    DsiTitleEntities_Clear();
    if (gMain.task == TASK_GAME) __real_EraseAllEntities();
}
#else
void EraseAllEntities(void) { DsiTitleEntities_Clear(); }
#endif
void UpdateEntities(void) { DsiTitleEntities_Update(); }
void DrawEntities(void) { DsiTitleEntities_Draw(); }
void DrawDirect(u32 s, u32 f) {
    if (!DsiTitleSprites_DrawDirect(s, f, &gOamCmd))
        ++gDsiTitleTrace.unsupported_service_calls;
}
void InitSoundPlayingInfo(void) { ++gDsiTitleTrace.unsupported_service_calls; }
void SoundReq(u32 sound) { (void)sound; ++gDsiTitleTrace.unsupported_service_calls; }
void BgAffineSet(struct BgAffineSrcData* src, struct BgAffineDstData* dst, s32 count) {
    (void)src; (void)dst; (void)count;
    ++gDsiTitleTrace.unsupported_service_calls;
}
void Port_TTS_Speak(const char* text, const PortTtsOptions* opts) {
    (void)text; (void)opts;
    ++gDsiTitleTrace.unsupported_service_calls;
}
