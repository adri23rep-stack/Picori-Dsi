# E46 original sprite upload

The generator selects src/vram.c's literal sub_080ADD70, sub_080ADE24 and
sub_080ADE74. Original scheduling, dirty state, forced refresh, destination and
slot clearing remain intact. DMA and LZ calls alone bind to DSi shadow-memory
transport; a DSi-only source guard prevents slot-table overruns.

DMA accepts registered stable E45 Link tile storage or bounded ROM spans.
It writes only GBA OBJ shadow 0x06010000..0x06017fff, never DS hardware directly.
Uncompressed ROM reads stage before writing; unsupported native pointers and
overflow abort. Compressed ROM uses the existing bounded LZ decoder after OBJ
length validation. Malformed compressed data can have partially written bytes
before abort (same decoder as existing E6); there is no success fallback.

Sanitizers: dirty/no-change frames, forced refresh, literal LZ data, foreign
pointers, BG writes, invalid slots and destination/table limits pass.
With the user's EU ROM, 4096 original animation ticks each upload exactly the
corresponding ROM tile bytes. run_sprite_upload_e46.sh [ROM_EU.gba].

This completes allocation -> original animation -> original upload dependency
tests, not the full player frame. Native linker retains the upload entry point.
Do not call it from a made-up player loop: integrate original WaitForNextFrame/
game dispatch after OAM, PlayerInit/PlayerSleep, scripts and remaining services.
The original sprite OAM builder and non-static display are still pending.
