# Original player init/sleep dependency audit
Host compilation only. Does not run or enable gameplay.
Reachability stops at externals outside player.c; it is not the full engine closure.
A symbol present in an ELF can still be a stub or blocked service.

## PlayerInit
Direct references: CheckQueuedAction, CreateItemEquippedAtSlot, DeleteClones, DeleteEntity, FindEntity, InitAnimationForceUpdate, IsItemEquipped, LoadSwapGFX, PlayerSetNormalAndCollide, PlayerSwimming, ResolveCollisionLayer, ResolvePlayerAnimation, UpdateFloorType, UpdatePlayerSkills, gPlayerEntity, gPlayerHitbox, gPlayerState, sub_0806FDA0
Reachable local functions: PlayerInit
External references (functions AND data):
- CheckQueuedAction
- CreateItemEquippedAtSlot
- DeleteClones
- DeleteEntity
- FindEntity
- InitAnimationForceUpdate
- IsItemEquipped
- LoadSwapGFX
- PlayerSetNormalAndCollide
- PlayerSwimming
- ResolveCollisionLayer
- ResolvePlayerAnimation
- UpdateFloorType
- UpdatePlayerSkills
- gPlayerEntity
- gPlayerHitbox
- gPlayerState
- sub_0806FDA0

## PlayerSleep
Direct references: .data.rel.ro.local.gUnk_0811BC88.2, CheckQueuedAction
Reachable local functions: PlayerSleep, Port_EntitySlot, Port_GetEntityScriptCtx, sub_08074C68, sub_08074CF8, sub_08074D34, sub_08074F00, sub_08074F1C, sub_08074F2C, sub_08074F44, sub_08074F8C, sub_08074FEC, sub_0807501C, sub_0807508C, sub_080750F4, sub_08075110, sub_0807513C, sub_0807518C, sub_080751B4
External references (functions AND data):
- CheckQueuedAction
- CreateSpeechBubbleExclamationMark
- CreateSpeechBubbleQuestionMark
- DeleteEntity
- ExecuteScript
- GravityUpdate
- LinearMoveUpdate
- SetFade
- SoundReq
- UpdateAnimationSingleFrame
- gActiveScriptInfo
- gAuxPlayerEntities
- gEntities
- gEntityScriptCtxTable
- gPlayerEntity
- gPlayerScriptExecutionContext
- gPlayerState
- sub_0807DD64
