/* Tests the ORIGINAL ReadKeyInput() compiled from src/common.c. */
#include "common.h"
#include "gba/io_reg.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
Input gInput;
uint8_t gIoMem[0x400];
int main(void) {
    REG_KEYINPUT=0x03ff;
    ReadKeyInput();
    assert(gInput.heldKeys==0 && gInput.newKeys==0);
    REG_KEYINPUT=0x03fe; /* Press A */
    ReadKeyInput();
    assert(gInput.heldKeys==1 && gInput.newKeys==1 && gInput.menuScrollKeys==1);
    ReadKeyInput();
    assert(gInput.heldKeys==1 && gInput.newKeys==0);
    REG_KEYINPUT=0x03fd; /* Switch from A to B */
    ReadKeyInput();
    assert(gInput.heldKeys==2 && gInput.newKeys==2);
    REG_KEYINPUT=0x03ff;
    ReadKeyInput();
    assert(gInput.heldKeys==0 && gInput.newKeys==0);
    puts("PASS: ORIGINAL ReadKeyInput input transitions");
    return 0;
}
