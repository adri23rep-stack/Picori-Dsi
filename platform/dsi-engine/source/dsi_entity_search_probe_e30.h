#ifndef DSI_ENTITY_SEARCH_PROBE_E30_H
#define DSI_ENTITY_SEARCH_PROBE_E30_H
#include <stdbool.h>
/* E30.11: 4 literal source functions from src/entity.c, executed only as
 * read-only searches on already initialized Zelda player list in DSi. */
void DsiEntitySearchProbeE30_Reset(void);
bool DsiEntitySearchProbeE30_Run(void);
bool DsiEntitySearchProbeE30_Passed(void);
#endif
