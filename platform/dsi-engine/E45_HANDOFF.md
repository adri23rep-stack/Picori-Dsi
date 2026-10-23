# E45 original Link animation dependency

port/port_animation.c contains the existing Thumb-to-C translation. The new
generator selects literal functions: InitializeAnimation, FrameZero, variable
and single frame updates, GetNextFrame, force update and GFX-slot marking.
Only ROM/native pointer transport is rebound. The source has DSi-only bounds
adapters for frame lookup and backward loop targets, avoiding invalid native
pointer arithmetic. No action, frame sequence or speed is invented.

Metadata comes from original data/gfx/link_animations.s, sprite_ptrs.s and
assets/assets.json. Link sprites 1..9 and 12 are supported, including sprite 7
sleep/no-cap sleep. Sprites 10/11 (Ezlo etc.) are deliberately unsupported.
The loader verifies every selected EU SpritePtr, animation-table terminator and
entry, frame bounds and tile span. Data stays in the user's ROM, never in git.

Native storage: 10668 bytes of contiguous original animation streams, frame
tables/offsets, and a single 581216-byte heap allocation for Link's shared
tiles. The contiguous stream region preserves original cross-label fallthrough.
Pointers are stable for the loaded session; room/ROM replacement needs a future
proper teardown after invalidating live entities. No eviction while animPtr is live.
Unsupported access aborts; preparation failure returns false without publishing
ready storage. Do not treat it as a fallback animation.

Boot now prepares data and displays Anim data:OK, but does not select/tick a
player animation. Original PlayerInit/PlayerSleep dispatch is still required.
E44 allocations are live; original animation entry points are retained in ARM9
to prove their link closure. Animation marks the genuine gGFXSlots for upload;
VRAM upload/OAM rendering is the next distinct dependency. Static Link display
remains static until the original player frame is enabled.

Tests: run_animation_e45.sh under ASan/UBSan covers pointer/read faults, malformed
tables/frames, timing, loops, GFX dirty marking, native pointer bounds and aborts.
With /path/to/EU.gba it compared 4599 actual Link animation ticks (standing,
walking, no cap, sleeping and waking) with independently read raw ROM frame records.
Both wake variants produce frame event 1 and ANIM_DONE; their terminal duration
is zero. PlayerSleep must consume ANIM_DONE and select the next state. Do not
drive the primitive indefinitely after completion or invent a looping wake.
Native CI must pass before calling this an ARM9 build; no emulator run yet.
