/* E40: fidelity/safety gate for the unverified E39 Archway sprite overlay.
 * E39 melonDS user screenshot proved the real original Archway action is
 * ACTIVE, the original static Link ROM sprite is visible, but the provisional
 * object compositor produces a bright magenta rectangular corruption above
 * the doorway. That compositor copies an ad-hoc OBJ bank/palette and is NOT
 * src/objectUtils.c::ObjectInit / src/vram.c::LoadFixedGFX nor the original
 * draw scheduler. It must not publish potentially false ROM pixels.
 *
 * The game engine is not modified: Archway() still runs with original ID
 * 0x4F and the real Link entity/RoomResInfo stays untouched. The fallback
 * is the authentic ROM BG1/BG2 room preview, NOT recreated sprite art.
 * Re-enable ONLY after porting native palette/VRAM allocation and validating
 * original OAM tile offsets with the EU retail ROM in melonDS. */
#include "dsi_room_sprite_policy_e40.h"
#include "object.h"
DsiRoomE40GfxDecision DsiRoomSpritePolicyE40_Archway(
    const DsiRoomMetadata *room,const Entity *arch) {
    if(!room || !arch || room->area!=0x22u || room->room!=0x15u ||
       arch->kind!=OBJECT || arch->id!=ARCHWAY || arch->action!=1 ||
       !arch->spriteSettings.draw || arch->frameIndex!=arch->type2)
        return DSI_E40_ARCH_GFX_INVALID;
    return DSI_E40_ARCH_GFX_WAITING_FOR_NATIVE_SPRITE_PIPELINE;
}
bool DsiRoomSpritePolicyE40_AllowPreview(const DsiRoomMetadata *room,const Entity *arch) {
    /* Explicitly fail-closed instead of passing through a known-corrupt
     * compositor. A future native sprite scheduler can replace this gate. */
    (void)DsiRoomSpritePolicyE40_Archway(room,arch);
    return false;
}
