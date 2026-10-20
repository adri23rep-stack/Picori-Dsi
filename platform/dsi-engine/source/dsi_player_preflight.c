/* E29.1: fail-closed verification for the ORIGINAL InitializePlayer.
 * The real implementation is in src/gameUtils.c. No fake Link object,
 * animation, input loop or replacement entity allocator here. */
#include "dsi_player_preflight.h"
#include "dsi_entity_lists.h"
#include "dsi_room_resources.h"
#include "game.h"
#include "player.h"
#include "room.h"
#include "save.h"

bool DsiPlayerPreflight_Ready(void) {
    const DsiEntityLists *lists = DsiEntityLists_Current();
    const DsiRoomResources *room = DsiRoomResources_Current();
    const PlayerRoomStatus *spawn = &gRoomTransition.player_status;
    if (!lists || !room || !lists->valid || !room->valid ||
        room->area != gRoomControls.area ||
        room->room != gRoomControls.room ||
        lists->area != room->area || lists->room != room->room ||
        spawn->area_next != room->area || spawn->room_next != room->room ||
        spawn->spawn_type > PL_SPAWN_FAST_TRAVEL ||
        spawn->layer > 3u ||
        gSave.stats.health == 0u ||
        gSave.stats.health > gSave.stats.maxHealth) return false;
    /* Source is the ROM EU and both entity record banks have verified
     * 16-byte terminators. Never dereference their GBA addresses as C. */
    for (unsigned bank = 0; bank < DSI_ENTITY_LIST_BANKS; ++bank) {
        if (lists->count[bank] >= DSI_ENTITY_RECORD_MAX ||
            lists->count[bank] != room->entity_counts[bank] ||
            lists->raw[bank][lists->count[bank]][0] != 0xFFu) return false;
    }
    return true;
}
