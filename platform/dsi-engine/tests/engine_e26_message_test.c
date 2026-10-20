/* E26 integration test: call the unmodified src/message.c function. */
#include "message.h"
#include "structures.h"
#include "common.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

Message gMessage;
TextRender gTextRender;
struct_02000040 gUnk_02000040;
static int original_called;
void MemClear(void *p, u32 size) { memset(p, 0, size); }
void __real_MessageInitialize(void);
void __wrap_MessageInitialize(void) {
    ++original_called;
    __real_MessageInitialize();
}
int main(void) {
    memset(&gMessage, 0x5a, sizeof gMessage);
    memset(&gTextRender, 0x5a, sizeof gTextRender);
    memset(&gUnk_02000040, 0x5a, sizeof gUnk_02000040);
    MessageInitialize();
    assert(original_called == 1);
    for (size_t i=0; i<sizeof gMessage; i++) assert(((u8*)&gMessage)[i] == 0);
    for (size_t i=0; i<sizeof gTextRender; i++) assert(((u8*)&gTextRender)[i] == 0);
    for (size_t i=0; i<4; i++) assert(((u8*)&gUnk_02000040)[i] == 0);
    puts("PASS E26: original MessageInitialize() cleared engine globals");
    return 0;
}
