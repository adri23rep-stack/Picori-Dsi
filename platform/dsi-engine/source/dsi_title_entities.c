/* E11: link the real src/object/titleScreenObject.c rather than replacing its
 * state machine.  A bounded, title-only allocation bridge isolates that code
 * until the original full entity allocator, sprite loader, and AI are ported. */
#include "dsi_title_entities.h"
#include "object.h"
#include "entity.h"
#include "main.h"
#include "port_rng.h"
#include "dsi_title_object_gfx.h"
#include "dsi_title_sprites.h"
#include <string.h>

#define DSI_TITLE_ENTITY_CAP 8u
static Entity sEntities[DSI_TITLE_ENTITY_CAP];
static unsigned sEntityCount;
static bool sEnabled;
DsiTitleEntityStats gDsiTitleEntityStats;
u32 gRand=PORT_RNG_SEED;
extern void TitleScreenObject(Entity *entity); /* ORIGINAL module */

void DsiTitleEntities_Enable(bool enabled) {
    sEnabled=enabled;
    DsiTitleEntities_Clear();
}

void DsiTitleEntities_Clear(void) {
    DsiTitleObjectGfx_Reset();
    memset(sEntities,0,sizeof(sEntities));
    sEntityCount=0;
    gRand=PORT_RNG_SEED;
    memset(&gDsiTitleEntityStats,0,sizeof(gDsiTitleEntityStats));
}

/* The complete CreateObject in src/objectUtils.c depends on the global GBA
 * allocator and entity-list manager; none of those are falsely claimed here.
 * Only the title-screen object is accepted, with matching original fields. */
Entity *CreateObject(Object type,u32 subtype,u32 param) {
    if(!sEnabled || type!=TITLE_SCREEN_OBJECT) {
        ++gDsiTitleEntityStats.unsupported;
        return NULL;
    }
    if(subtype>=5u || sEntityCount>=DSI_TITLE_ENTITY_CAP) {
        ++gDsiTitleEntityStats.errors;
        return NULL;
    }
    Entity *ent=&sEntities[sEntityCount];
    memset(ent,0,sizeof(*ent));
    ent->kind=OBJECT;
    ent->id=(u8)type;
    ent->type=(u8)subtype;
    ent->type2=(u8)param;
    if (!DsiTitleObjectGfx_InitEntity(ent)) {
        ++gDsiTitleEntityStats.errors;
        return NULL;
    }
    ++sEntityCount;
    ++gDsiTitleEntityStats.created;
    ++gDsiTitleEntityStats.subtype_counts[subtype];
    gDsiTitleEntityStats.alive=sEntityCount;
    return ent;
}

/* Snapshot count: children created this tick are visited next tick.  This
 * avoids recursion and is consistent with the normal entity update pass. */
void DsiTitleEntities_Update(void) {
    if(!sEnabled) return;
    const unsigned before=sEntityCount;
    for(unsigned i=0;i<before;i++) {
        Entity *ent=&sEntities[i];
        if(ent->id!=TITLE_SCREEN_OBJECT || ent->type>=5u) {
            ++gDsiTitleEntityStats.errors;
            continue;
        }
        TitleScreenObject(ent); /* unmodified original object behavior */
        ++gDsiTitleEntityStats.updated;
        if(ent->type==0) ++gDsiTitleEntityStats.root_updates;
        else ++gDsiTitleEntityStats.child_updates;
    }
}

/* Uses original titleScreenObject.c entity position, frame index and
 * gObjectDefinition_BD sprite index. Only normal non-affine OAM transport is
 * supported. This is not the complete game's DrawEntities() pipeline. */
void DsiTitleEntities_Draw(void) {
    if(!sEnabled) return;
    for(unsigned i=0;i<sEntityCount;i++) {
        Entity *ent=&sEntities[i];
        if(!ent->spriteSettings.draw || ent->frameIndex==0xffu) continue;
        OAMCommand cmd={0};
        cmd.x=(int16_t)ent->x.HALF.HI;
        cmd.y=(int16_t)ent->y.HALF.HI;
        cmd._8=(uint16_t)(ent->spriteVramOffset | ((uint16_t)(ent->palette.raw&15u)<<12));
        if(!DsiTitleSprites_DrawDirect(ent->spriteIndex,ent->frameIndex,&cmd))
            ++gDsiTitleEntityStats.errors;
    }
}

/* This is the GBA Random() algorithm (ARM 0x08000E50), shared with the
 * existing PC port via port_rng.h. We preserve the original seed and output. */
u32 Random(void) {
    gRand=Port_Rng_Advance(gRand);
    return Port_Rng_Output(gRand);
}
