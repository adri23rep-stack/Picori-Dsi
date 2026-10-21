/* E29.8: assert exact source-translated Thumb sprite layer rules. */
#include "entity.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void UpdateSpriteForCollisionLayer(Entity *);
int main(void) {
    Entity e;
    for (unsigned i=0;i<4;++i) {
        memset(&e,0,sizeof e);
        e.collisionLayer=i;
        UpdateSpriteForCollisionLayer(&e);
        assert(e.spriteRendering.b3 == (i >= 2 ? 1u : 2u));
        assert(e.spriteOrientation.flipY == (i >= 2 ? 1u : 2u));
    }
    UpdateSpriteForCollisionLayer(NULL);
    puts("PASS E29.8: original Thumb sprite layer translated by existing 3DS port");
    return 0;
}
