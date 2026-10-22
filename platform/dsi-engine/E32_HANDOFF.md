# E32 — original entity priority scheduler

Branch `e32-native-priority-core` from E31.1 (commit `85b8c419e931313c4afc13110f4c5dd91d7070f0`).

- Priority gating, message blocking, event requests, timers and reset
  are extracted *verbatim* from original `src/entity.c`; source unchanged.
- `DsiPriorityE32_Tick` only calls original static `UpdatePriority`.
  Not used by the DSi frame loop before actual entity dispatch is ready.
- DSi boot calls original `EntityDisabled` and `AnyPrioritySet` read-only
  after E31 persistent room load. Does not allow Link control or movement.
- Host tests: `E32_SANITIZE=1 sh platform/dsi-engine/tests/run_priority_e32.sh`
- CI also verifies E30/E31 regressions and native ARM9 symbols.

**NOT PLAYABLE YET.** Missing original GameMain_ChangeRoom/UPDATE, entity
and manager callbacks (manager id 0x0a -> EzloHintManager_Main), scripts,
sprite/OAM live renderer, collision and transitions. Do not fake the
simulation, run unbound GBA callbacks, or label compiler PASS as playable.
No ROM should be stored in GitHub. Validate in melonDS and DSi hardware.
