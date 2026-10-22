/* Tests call original functions; expected values are test oracles, not gameplay. */
#include "common.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
Input gInput;
PlayerState gPlayerState;
extern void UpdatePlayerInput(void);
extern u32 ConvInputToState(u32);
static u32 expected(u32 keys) {
    u32 s=0;
    if(keys&A_BUTTON)s|=INPUT_INTERACT|INPUT_40|INPUT_USE_ITEM1;
    if(keys&B_BUTTON)s|=INPUT_CANCEL|INPUT_USE_ITEM2;
    if(keys&R_BUTTON)s|=INPUT_CONTEXT|INPUT_LIFT_THROW|INPUT_ACTION;
    if(keys&L_BUTTON)s|=INPUT_FUSE;
    if(keys&DPAD_RIGHT)s|=INPUT_RIGHT;
    if(keys&DPAD_LEFT)s|=INPUT_LEFT;
    if(keys&DPAD_UP)s|=INPUT_UP;
    if(keys&DPAD_DOWN)s|=INPUT_DOWN;
    return s;
}
int main(void) {
    static const u8 directions[16]={255,8,24,255,0,4,28,255,16,12,20,255,255,255,255,255};
    for(unsigned keys=0;keys<1024;keys++) {
        memset(&gPlayerState,0,sizeof gPlayerState);
        gInput.heldKeys=keys;
        assert(ConvInputToState(keys)==expected(keys));
        gPlayerState.controlMode=CONTROL_ENABLED;
        UpdatePlayerInput();
        assert(gPlayerState.playerInput.heldInput==expected(keys));
        assert(gPlayerState.playerInput.newInput==expected(keys));
        assert(gPlayerState.direction==directions[(keys>>4)&15]);
        UpdatePlayerInput();assert(!gPlayerState.playerInput.newInput);
        gInput.heldKeys=0;UpdatePlayerInput();
        assert(!gPlayerState.playerInput.heldInput&&!gPlayerState.playerInput.newInput);
        assert(gPlayerState.direction==255);
        gInput.heldKeys=keys;gPlayerState.controlMode=CONTROL_DISABLED;
        UpdatePlayerInput();assert(!gPlayerState.playerInput.heldInput);
        assert(gPlayerState.controlMode==CONTROL_DISABLED);
        gPlayerState.controlMode=CONTROL_2;
        UpdatePlayerInput();assert(!gPlayerState.playerInput.heldInput);
        gPlayerState.controlMode=CONTROL_1;
        UpdatePlayerInput();assert(gPlayerState.controlMode==CONTROL_ENABLED);
        assert(gPlayerState.playerInput.heldInput==expected(keys));
        assert(!gPlayerState.playerInput.newInput); /* no synthetic press on resume */
    }
    /* Macro wins over physical buttons and disabled player control. */
    PlayerMacroEntry macro[]={{2,DPAD_RIGHT},{1,A_BUTTON},{0xc000,0}};
    memset(&gPlayerState,0,sizeof gPlayerState);
    gPlayerState.controlMode=CONTROL_DISABLED;gInput.heldKeys=B_BUTTON;
    InitPlayerMacro(macro);
    UpdatePlayerInput();assert(gPlayerState.playerInput.heldInput==INPUT_RIGHT);
    UpdatePlayerInput();assert(gPlayerState.playerInput.heldInput==INPUT_RIGHT);
    assert(!gPlayerState.playerInput.newInput);
    UpdatePlayerInput();assert(gPlayerState.playerInput.heldInput==expected(A_BUTTON));
    UpdatePlayerInput();assert(!gPlayerState.playerInput.playerMacro);
    assert(!gPlayerState.playerInput.heldInput);
    UpdatePlayerInput();assert(!gPlayerState.playerInput.heldInput);
    /* Jump skips an entry; IGNORE preserves held keys for this frame. */
    PlayerMacroEntry jump[]={{0x4000,2},{1,B_BUTTON},{1,L_BUTTON},{0x8000,0},{0xc000,0}};
    InitPlayerMacro(jump);
    UpdatePlayerInput();assert(gPlayerState.playerInput.heldInput==INPUT_FUSE);
    UpdatePlayerInput();assert(gPlayerState.playerInput.heldInput==INPUT_FUSE);
    /* IGNORE decrements zero to 0xffff in the original engine: preserve it. */
    assert(gPlayerState.playerInput.playerMacroWaiting==0xffff);
    InitPlayerMacro(NULL);UpdatePlayerInput();assert(!gPlayerState.playerInput.heldInput);
    puts("E31.1 original input: 1024 key combinations, edges, control modes, macros PASS");
    return 0;
}
