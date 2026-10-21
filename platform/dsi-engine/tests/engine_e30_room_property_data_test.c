/* E30.2: original 16-byte EntityData materialization without spawn.
 * Source fixtures in this host test are synthetic, not copyrighted ROM. */
#include "dsi_room_property_data_e30.h"
#include "dsi_room_properties_e30.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static DsiRoomPropertiesE30 props;
static DsiEntityLists lists;
const DsiRoomPropertiesE30 *DsiRoomPropertiesE30_Current(void) {
    return props.valid ? &props : NULL;
}
const DsiEntityLists *DsiEntityLists_Current(void) {
    return lists.valid ? &lists : NULL;
}
static void fixture(void) {
    memset(&props,0,sizeof props);
    memset(&lists,0,sizeof lists);
    props.valid=lists.valid=true;
    props.area=lists.area=0x22;
    props.room=lists.room=0x15;
    props.kind[0]=props.kind[1]=DSI_E30_PROPERTY_ROM_DATA;
    props.original[0]=lists.gba_address[0]=0x08001000u;
    props.original[1]=lists.gba_address[1]=0x08002000u;
    lists.count[0]=2; lists.count[1]=1;
    lists.raw[0][0][0]=3; lists.raw[0][0][1]=0x12;
    lists.raw[0][0][4]=0x44; lists.raw[0][0][5]=0x33;
    lists.raw[0][0][6]=0x22; lists.raw[0][0][7]=0x11;
    lists.raw[0][0][8]=0x34; lists.raw[0][0][9]=0x12;
    lists.raw[0][1][0]=7;
    lists.raw[0][2][0]=0xFF;
    lists.raw[1][0][0]=6;
    lists.raw[1][1][0]=0xFF;
}
int main(void) {
    DsiRoomPropertyDataE30_Reset();
    assert(!DsiRoomPropertyDataE30_Ready());
    assert(DsiRoomPropertyDataE30_EntityBank(0)==NULL);
    fixture();
    assert(DsiRoomPropertyDataE30_Bind());
    assert(DsiRoomPropertyDataE30_Ready());
    const EntityData *a=DsiRoomPropertyDataE30_EntityBank(0);
    const EntityData *b=DsiRoomPropertyDataE30_EntityBank(1);
    assert(a && b && a[0].kind==3 && a[1].kind==7 && a[2].kind==0xFF);
    assert(b[0].kind==6 && b[1].kind==0xFF);
    assert(a[0].type2==0x11223344u && a[0].xPos==0x1234u);
    assert(((uintptr_t)a % _Alignof(EntityData))==0);
    assert(DsiRoomPropertyDataE30_Count(0)==2 &&
           DsiRoomPropertyDataE30_Count(1)==1);
    assert(DsiRoomPropertyDataE30_EntityBank(2)==NULL);
    assert(DsiRoomPropertyDataE30_Count(2)==0u);
    /* Source mutation invalidates live reads: no dangling/stale bank. */
    lists.raw[0][0][1]=0x55;
    assert(!DsiRoomPropertyDataE30_Ready());
    assert(DsiRoomPropertyDataE30_EntityBank(0)==NULL);
    lists.raw[0][0][1]=0x12;
    assert(DsiRoomPropertyDataE30_Ready());
    /* Malformed bank refuses publication and preserves previous copy. */
    lists.raw[0][1][0]=0xFF;
    assert(!DsiRoomPropertyDataE30_Bind());
    lists.raw[0][1][0]=7;
    assert(DsiRoomPropertyDataE30_Ready());
    props.original[1]=0x0800F000u;
    assert(!DsiRoomPropertyDataE30_Bind());
    assert(!DsiRoomPropertyDataE30_Ready());
    props.original[1]=lists.gba_address[1];
    assert(DsiRoomPropertyDataE30_Ready());
    assert(DsiRoomPropertyDataE30_EntityBank(0)==a);
    /* Wrong room and stale snapshots must not be usable. */
    props.room=0x16;
    assert(!DsiRoomPropertyDataE30_Ready());
    props.room=0x15;
    assert(DsiRoomPropertyDataE30_Bind());
    DsiRoomPropertyDataE30_Reset();
    assert(!DsiRoomPropertyDataE30_Ready());
    puts("PASS E30.2: original EntityData banks aligned, immutable and guarded");
    return 0;
}
