/* E34 verifies exact ORIGINAL compiled EU bed definition, not hardcoded art. */
#include "definitions.h"
#include "hitbox.h"
/* Host-only link fixtures for unused other object hitbox references. */
const Hitbox gHitbox_0={0},gHitbox_2={0},gHitbox_3={0},gHitbox_30={0};
#include <assert.h>
#include <stdio.h>
extern const ObjectDefinition gObjectDefinition_4F[];
int main(void) {
    const ObjectDefinition* d=&gObjectDefinition_4F[1];
    assert(d->bitfield.type==1);
    assert(d->bitfield.gfx_type==0);
    assert(d->bitfield.gfx==99);
    assert(d->data.sprite.paletteIndex==10);
    assert(d->data.sprite.spriteIndex==407);
    assert(d->data.sprite.draw==1);
    puts("E34 EU original BedCover type1: GFX99 pal10 sprite407 PASS");
}
