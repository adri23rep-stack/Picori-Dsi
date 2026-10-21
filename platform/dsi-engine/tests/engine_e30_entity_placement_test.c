/* This is the ORIGINAL src/room.c placement logic executed on host,
 * not a DSi-specific rewrite. Script mode 0x40 must remain blocked. */
#include "room.h"
#include "enemy.h"
#include "script.h"
#include "common.h"
#include "port/port_rom.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

RoomControls gRoomControls;
u8 *gRomData;
u32 gRomSize;
u32 Port_TranslateScriptAddr(u32 gba_addr) {
    (void)gba_addr;
    assert(!"unported script resolution must never execute");
    return 0u;
}
ScriptExecutionContext *StartCutscene(Entity* e, Script* script) {
    (void)e; (void)script;
    assert(!"unported GBA script must never execute");
    return NULL;
}
void DeleteEntity(Entity *e) {
    (void)e;
    assert(!"unported DeleteEntity must never execute");
}
extern void sub_0804AF0C(Entity *, const EntityData *);
int main(void) {
    EntityData d={0};
    Entity npc={0};
    Enemy enemy={0};
    gRoomControls.origin_x=128;
    gRoomControls.origin_y=200;
    d.kind=NPC; d.flags=0; d.xPos=12; d.yPos=35;
    sub_0804AF0C(&npc,&d);
    assert(npc.x.HALF.HI==140 && npc.y.HALF.HI==235);
    d.kind=ENEMY; d.flags=0x20;
    d.xPos=25; d.yPos=40;
    sub_0804AF0C(&enemy.base,&d);
    assert(enemy.base.x.HALF.HI==153 && enemy.base.y.HALF.HI==240);
    assert(enemy.enemyFlags & EM_FLAG_CAPTAIN);
    npc.x.HALF.HI=42; npc.y.HALF.HI=73;
    d.flags=0x10;
    sub_0804AF0C(&npc,&d);
    assert(npc.x.HALF.HI==42 && npc.y.HALF.HI==73);
    puts("PASS E30.9: unchanged original sub_0804AF0C sets room-relative coordinates/captain flag; script branch remains excluded");
    return 0;
}
