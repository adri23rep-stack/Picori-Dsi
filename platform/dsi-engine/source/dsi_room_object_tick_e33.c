/* E33: run the REAL BedCover object from src/object/bedCover.c.
 * This file implements no Zelda movement, object AI or script interpreter.
 * Reject all unsupported entity ids, scripted variants and transitions. */
#include "dsi_room_object_tick_e33.h"
#include "dsi_room_spawn_e31.h"
#include "entity.h"
#include "object.h"
#include "player.h"
#include "manager.h"
#include "main.h"
#include "game.h"
#include "room.h"
#include "script.h"
#include <stdlib.h>
static Entity* sBed;
static unsigned sTicks,sErrors;
void DsiRoomObjectE33_Reset(void) {
    sBed=NULL; sTicks=sErrors=0;
}
bool DsiRoomObjectE33_Ready(void) {
    return sTicks>0 && sErrors==0 && sBed!=NULL &&
        sBed->kind==OBJECT && sBed->id==BED_COVER &&
        sBed->action==1 && sBed->spriteSettings.draw==1 &&
        sBed->frameIndex==1;
}
unsigned DsiRoomObjectE33_Ticks(void) { return sTicks; }
unsigned DsiRoomObjectE33_Errors(void) { return sErrors; }
/* Fail loudly if a future room enables scripted objects before Zelda's
 * original ExecuteScript interpreter is linked and bound. Never pretend
 * that scripts succeeded. --wrap=ExecuteScript only catches this path. */
void __wrap_ExecuteScript(Entity* e, ScriptExecutionContext* ctx) {
    (void)e; (void)ctx;
    ++sErrors;
    abort();
}
bool DsiRoomObjectE33_Tick(void) {
    if (!DsiRoomSpawnE31_Ready() || sErrors) return false;
    if (gMain.task!=TASK_GAME || gMain.state!=GAMETASK_MAIN ||
        gMain.substate!=GAMEMAIN_CHANGEROOM ||
        gRoomControls.area!=0x22 || gRoomControls.room!=0x15 ||
        gEntCount!=2 || gManagerCount!=3 ||
        gPlayerEntity.base.kind!=PLAYER) {
        ++sErrors; return false;
    }
    Entity* bed=DeepFindEntityByID(OBJECT, BED_COVER);
    if (!bed || bed->type!=1 || (bed->flags&(ENT_DELETED|ENT_SCRIPTED)) ||
        bed->action>1 || (sBed && sBed!=bed)) {
        ++sErrors; return false;
    }
    sBed=bed;
    if (EntityDisabled(bed)) return DsiRoomObjectE33_Ready();
    /* Authentic src/object/bedCover.c action dispatch and animation setup. */
    BedCover(bed);
    if (bed->action!=1 || bed->spriteSettings.draw!=1 ||
        bed->frameIndex!=1 || bed->kind!=OBJECT ||
        gEntCount!=2 || gManagerCount!=3) {
        ++sErrors; return false;
    }
    ++sTicks;
    return true;
}
