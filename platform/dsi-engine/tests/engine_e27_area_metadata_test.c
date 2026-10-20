/* E27: source-level verification of ORIGINAL Picori area tables. */
#include "area.h"
#include "game.h"
#include "flags.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(gAreaMetadata[AREA_HOUSE_INTERIORS_2].flag_bank == LOCAL_BANK_2);
    assert(gAreaMetadata[AREA_HOUSE_INTERIORS_2].flags & AR_HAS_ENEMIES);
    assert(gAreaMetadata[AREA_MINISH_WOODS].flags & AR_IS_OVERWORLD);
    unsigned count=0;
    while(count<64u && gOverworldLocations[count].minX!=0xFFu){
        assert(gOverworldLocations[count].maxX >= gOverworldLocations[count].minX);
        ++count;
    }
    assert(count==17u && gOverworldLocations[count].minX==0xFFu);
    puts("PASS E27: original gAreaMetadata and full gOverworldLocations table");
    return 0;
}
