# E44 original sprite-slot allocation

Selected literal src/vram.c functions now provide ResetPalettes, LoadSwapGFX,
allocation, reservation and compaction; src/physics.c provides sub_0806FD8C and
sub_0806FDA0. Existing PC_PORT fixes are retained. Storage is the existing
gGFXSlots and gUnk_020000C0, not duplicate buffers.

Native E44 replaces the diagnostic ResetPalettes and sub_0806FD8C definitions.
Existing initialization calls now perform the actual resets. The original
PlayerInit resource allocation entries link but PlayerInit is not yet dispatched.
Legacy host probes retain their old diagnostics unless E44 is enabled.

Sanitizer test run_sprite_slots_e44.sh passes: exact PlayerInit slot 2 and extra
sprite allocation, all 47 extra entries, exhaustion/reuse, all 40 dynamic GFX
slots, malformed requests, cleanup and compaction across active entity lists,
extra sprites and OAM. Suspended entities stay untouched by compaction.
Unload uses the original lifecycle E31 functions, not test success stubs.

This is allocation bookkeeping only. Animated ROM data, upload and full player
dispatch are separate dependencies; do not advertise a playable build.
