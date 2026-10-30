# Base damage logging

## Scope and status

This implementation emits `basebuilding_damage` only for a hit that changes a `BaseBuildingBase` or `BuildingBase` entity, or one of its damage zones, to ruined on the server. Ordinary hits, initialization and further hits on already ruined zones are ignored. It uses the existing ServerSentry event queue and JSONL writer.

The patch has been reviewed against the uploaded project and Bohemia's script declarations. It has **not** been compiled with the DayZ toolchain or exercised on a running DayZ server. Complete the native checks below before distributing a PBO.

The event records damage observations. It does not prove a raid succeeded, establish base ownership, count unique projectiles/explosions, or calculate exact removed HP. Mod-specific scripts that directly change health or construction state without invoking this callback need their own adapter.

## Enable the feature

Add or update these properties in the existing `$profile:ServerSentry/config.json`, with the server stopped:

```json
{
  "EnableTelemtryLogging": true,
  "LogBaseDamage": true
}
```

This is a configuration fragment, not a replacement for the complete file. Preserve other settings and the existing spelling of `EnableTelemtryLogging`.

`LogBaseDamage` defaults to `false`, matching the opt-in nature of existing construction logs. The existing configuration migration adds missing fields while preserving explicitly configured values. This additive flag does not change the meaning of an existing field, so the configuration version remains unchanged.

`LogBaseBuilding`, `LogActionDismantlePart` and `LogActionDestroyPart` remain independent action settings.

## Code structure

| File | Responsibility |
| --- | --- |
| `scripts/4_World/hooks/BaseBuildingBase.c` | Observes accepted damage and ruined transitions on both base classes, then submits only the qualifying hit. Always preserves inherited behavior. |
| `scripts/4_World/Logging/ServerSentryBaseDamageFactory.c` | Applies server/config guards and copies source, target, damage and attribution values. |
| `scripts/3_Game/models/Logging/ServerSentryBaseDamageData.c` | Plain data and per-zone DTOs; no world/entity references. |
| `scripts/3_Game/models/Logging/ServerSentryBaseDamageEvent.c` | Typed event wrapper. |
| `scripts/3_Game/Logging/ServerSentryLogger.c` | Dispatches to the typed serializer and existing bounded queue. |
| `scripts/3_Game/models/Config/ServerSentryLoggingConfig.c` | Adds the opt-in flag. |
| `scripts/3_Game/ConfigManager.c` | Exposes the category flag. |

Keep the data classes in `3_Game` and world-entity logic in `4_World`. No new addon dependency or script-module path is required in the supplied `config.cpp`.

`EEOnDamageCalculated` starts a fresh observation only when the inherited decision accepts damage and logging is enabled. Its return value is preserved. `EEHealthLevelChanged` marks an actual transition from an existing non-ruined health level to `STATE_RUINED`, before inherited construction cleanup can remove or reset the zone. `EEHitBy` consumes that observation and builds an event only when such a transition occurred. This includes ruined construction zones even when global health remains positive.

The qualifying hook copies values before its `super.EEHitBy` call. This is not a pre-damage health measurement: engine damage has already been applied. State is cleared on every damage calculation and hit callback, so observations are not reused for subsequent hits. The snapshot contains no `TotalDamageResult`, player or target references; there is no delayed access to engine-owned callback data. Mods must preserve all three inherited callbacks for this correlation to work.

## Event contract

- `eventType`: `basebuilding_damage`.
- `category`: `basebuilding`.
- `action.name`: `basebuilding_damage`.
- `action.result`: `observed`.
- `severity`: `info`.
- `target`: base entity, with `type: object` and `entityKind: basebuilding`.
- `position` and `targetPosition`: target world position.
- `actorPosition`: present only if a player entity was found.
- `data`: a JSON object serialized through `JsonFileLoader<ServerSentryBaseDamageEvent>`.
- `dataJson`: empty, following the existing typed-event convention.

| Data field | Meaning |
| --- | --- |
| `sourceHook` | `BaseBuildingBase.EEHitBy` or `BuildingBase.EEHitBy`. |
| `targetId` | Runtime `GetID()` of the target. Scope it to server and boot; it is not a persistent base/owner ID. |
| `damageType` | Original numeric DayZ `DamageType` value. |
| `damageKind` | `firearm`, `explosion`, `melee`, `stun`, `custom`, or `other`, derived from the enum. |
| `component` | Original engine hit-component index. |
| `damageZone` | Original callback zone, including an empty string. It is not assumed to list every damaged zone. |
| `ammoType` | Original ammunition/damage configuration name. |
| `sourceClassName` | Physical source class when available. It is not necessarily a weapon or a player. |
| `attackerResolution` | Attribution method described below. |
| `explosive` | Shared explosive provenance used by combat hits and kills; null for other damage. |
| `globalHealthDamage` | Positive `GetDamage("", "Health")`, otherwise zero. |
| `highestHealthDamage` | Positive `GetHighestDamage("Health")`, otherwise zero. This is a maximum, not a sum. |
| `modelPosition` | Original callback position in the target's model coordinates. |
| `zones` | Named zones with positive `GetDamage(zoneName, "Health")`; each item has `zoneName` and `healthDamage`. |

All damage quantities are copied engine results, not reconstructed before/after health deltas. Do not add global damage to named-zone damage as if these were independent losses. Do not treat `highestHealthDamage` as total explosion damage. An event with positive reported damage can have an empty `zones` array when no named zone can be resolved.

The factory enumerates only the receiving object's configured damage zones. It does not scan the world. It also checks a non-empty callback zone if a mod omitted it from that enumeration. It preserves the raw callback name instead of inventing a construction-part name from geometry or position.

## Attacker attribution

| `attackerResolution` | Interpretation |
| --- | --- |
| `source_player` | The source object itself is a `PlayerBase` with a current identity. |
| `hierarchy_root_player` | For firearm/melee damage, a player with identity is the source's current hierarchy root. |
| `player_entity_without_identity` | A player entity was found, but its identity was unavailable. No IDs are fabricated. |
| `unresolved` | No supported player attribution was available. The damage observation is still emitted. |
| `detonated_by` | Explicit remote operator or actor whose damage caused the detonation. |
| `released_by` | Verified player who released the live grenade from their hands. |
| `armed_by` | Recorded arming actor, including persistent mines and trap controllers. |
| `source_weapon_player` | Explosion supplied a firing weapon as its source, as with grenade-launcher damage. |

For an unresolved actor, `actor.type` and `actor.entityKind` are `unknown`, and identity fields remain empty. Missing identity is not automatically classified as AI.

Physical explosive sources do not use their current inventory owner as the attacker. A grenade may already be on the ground, or it may explode in a carrier's inventory due to another cause. The current holder is not proof of the initiator. A mod explicitly supplying a player as the explosion source can still use the direct-player path.

The shared `ServerSentryExplosiveResolver` now captures verified roles from grenade pin/release operations, trap placement/arming and remote activation. IED and tripwire provenance is copied to the actual charges before activation. Device roles persist through CF ModStorage without extending vanilla item data. The triggering victim is recorded separately and never selected as the responsible actor. See [Explosive attribution](EXPLOSIVE_ATTRIBUTION.md) for the shared hit/kill/base-damage contract and remaining integration limits.

## Damage, destruction and actions

`EEHitBy` supplies the damage observation. Do not fabricate `ActionData` or route it through the existing build-action lifecycle: that lifecycle expects a player action target and has a 500 ms deduplication window.

The filter uses actual ruined transitions rather than a time-based deduplication window. It emits at most one event per qualifying hit callback, even if that hit ruins multiple zones. Rebuilding or repairing a zone permits a later hit to ruin it again and produce a new event. Event count is not a guaranteed bullet or explosion count.

`EEOnDamageCalculated` is a pre-application decision hook. Its calculated result alone never produces an event: both a ruined health transition and the subsequent hit callback are required. Damage rejected by inherited logic is not tracked.

## Consumer and performance integration

The existing logger assigns event IDs, sequence numbers and boot metadata, serializes to JSON, then buffers output. The hook does not open files, flush for each hit, send HTTP requests or emit additional client RPCs. Serialization remains synchronous and should be measured under real raid load.

The queue's existing limits and dropped-event reporting still apply. This is not a lossless audit guarantee under process crashes, queue overflow or persistent disk errors.

Update any downstream event allowlist/schema to accept `basebuilding_damage`, the typed `data.zones` array, and `action.result: observed`. If records are uploaded again, use the original `eventId` for idempotency. A timestamp/source/ammo combination is not a safe duplicate key for rapid hits. This patch does not modify an external Agent, CMS, dashboard or alert rule.

Non-final hits are intentionally discarded before event allocation, zone enumeration or serialization. The event is still a hit observation, not a measured exact HP-loss total or a proof that a raid succeeded.

## Native validation

Run these checks on the actual DayZ version and mod combination, using the normal PBO build and a test server. No native test was executed while preparing this patch.

| Scenario | Expected observation |
| --- | --- |
| Build and start | Enforce modules compile; no missing symbol, duplicate override or serializer error. |
| Non-final shot into a built fence zone | No `basebuilding_damage` event. |
| Fast automatic fire | Only callbacks that actually ruin a zone produce an event; no per-hit debug prints. |
| Two simultaneous attackers | Each observation uses its own source; there is no shared last-attacker state. |
| Ground grenade, mine or IED | Only ruined-transition hits are retained with physical source/ammo; unresolved attribution has no invented Steam ID. |
| Explosive in a player's inventory | A current carrier is not automatically assigned as the attacker. |
| Multiple affected zones/objects | At most one event per qualifying callback; events are not interpreted as unique explosions. |
| Final hit destroys a part | One event is recorded even if global health remains positive or construction cleanup resets the zone. |
| More hits on an already ruined zone | No new event until repair/rebuilding and another actual ruined transition. |
| `source = null` or missing identity | A qualifying ruined-transition hit is retained with unknown/qualified attribution. |
| Zero/non-positive Health damage | No base-damage record is emitted. |
| Telemetry disabled, then category disabled | Either switch suppresses this event path independently. |
| Vanilla base damage disabled / raid-protection mod | Verify actual callback behavior and zero/blocked results; no damage-setting changes are introduced by this logger. |
| Other base/raid mods | Confirm subclasses call the inherited hook; direct state mutations need a separate adapter. |
| Sustained raid load and restart | Inspect queue/dropped-event markers and server timing; treat `targetId` as session-scoped. |
| Agent/CMS consumption | New event type, `observed` result and typed zone list are accepted and displayed correctly. |

## Official references

The implementation was checked against these Bohemia source files on 2026-10-27. `main` is a moving branch; validate against the server build used for deployment.

- [DamageSystem: native result accessors and DamageType enum](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/3_game/damagesystem.c).
- [Object: damage-zone enumeration, pre-application callback and physical explosion source](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/3_game/entities/object.c).
- [EntityAI: hit callback and current hierarchy-root lookup](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/3_game/entities/entityai.c).
- [BaseBuildingBase: construction-part callbacks](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/entities/itembase/basebuildingbase.c).
- [ConstructionBase: ruined-zone handling and null-player destruction](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/classes/constructionbase.c).
- [ImprovisedExplosive: detached charge activation](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/entities/explosivesbase/improvisedexplosive.c).
