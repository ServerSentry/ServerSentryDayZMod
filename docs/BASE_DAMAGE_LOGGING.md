# Base damage logging

## Scope and status

This implementation observes positive Health damage results delivered to `BaseBuildingBase.EEHitBy` on the server. It covers inherited callbacks on vanilla base entities and compatible mod subclasses. It uses the existing ServerSentry event queue and JSONL writer.

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
| `scripts/4_World/hooks/BaseBuildingBase.c` | Captures the callback, always calls inherited behavior once, then submits the owned event. |
| `scripts/4_World/Logging/ServerSentryBaseDamageFactory.c` | Applies server/config guards and copies source, target, damage and attribution values. |
| `scripts/3_Game/models/Logging/ServerSentryBaseDamageData.c` | Plain data and per-zone DTOs; no world/entity references. |
| `scripts/3_Game/models/Logging/ServerSentryBaseDamageEvent.c` | Typed event wrapper. |
| `scripts/3_Game/Logging/ServerSentryLogger.c` | Dispatches to the typed serializer and existing bounded queue. |
| `scripts/3_Game/models/Config/ServerSentryLoggingConfig.c` | Adds the opt-in flag. |
| `scripts/3_Game/ConfigManager.c` | Exposes the category flag. |

Keep the data classes in `3_Game` and world-entity logic in `4_World`. No new addon dependency or script-module path is required in the supplied `config.cpp`.

The hook copies values before its `super.EEHitBy` call so later inherited behavior cannot change the data being submitted. This is not a pre-damage health measurement. The snapshot contains no `TotalDamageResult`, player or target references; there is no delayed access to engine-owned callback data.

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
| `sourceHook` | `BaseBuildingBase.EEHitBy`. |
| `targetId` | Runtime `GetID()` of the target. Scope it to server and boot; it is not a persistent base/owner ID. |
| `damageType` | Original numeric DayZ `DamageType` value. |
| `damageKind` | `firearm`, `explosion`, `melee`, `stun`, `custom`, or `other`, derived from the enum. |
| `component` | Original engine hit-component index. |
| `damageZone` | Original callback zone, including an empty string. It is not assumed to list every damaged zone. |
| `ammoType` | Original ammunition/damage configuration name. |
| `sourceClassName` | Physical source class when available. It is not necessarily a weapon or a player. |
| `attackerResolution` | Attribution method described below. |
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

For an unresolved actor, `actor.type` and `actor.entityKind` are `unknown`, and identity fields remain empty. Missing identity is not automatically classified as AI.

Physical explosive sources do not use their current inventory owner as the attacker. A grenade may already be on the ground, or it may explode in a carrier's inventory due to another cause. The current holder is not proof of the initiator. A mod explicitly supplying a player as the explosion source can still use the direct-player path.

For reliable explosive attribution, add an explicit integration that captures released-by, placed-by, armed-by or triggered-by identities at the corresponding server operation. Copy identities at that time, preserve the relationship type and propagate it to actual IED charge objects. A generic last-holder cache or nearest-player lookup is insufficient. This extra integration is not included in this patch.

## Damage, destruction and actions

`EEHitBy` supplies the damage observation. Do not fabricate `ActionData` or route it through the existing build-action lifecycle: that lifecycle expects a player action target and has a 500 ms deduplication window.

The patch intentionally has no time-based hit deduplication. Rapid hits must remain distinct observations. It emits at most one event for each callback it receives, with multiple zone entries inside that event. The engine can invoke callbacks for multiple targets or zones; event count is not a guaranteed bullet or explosion count.

The separate `basebuilding_part_destroyed` event is now available through `LogBaseDestruction`. It wraps `Construction.OnHealthLevelChanged` and confirms a built-to-unbuilt transition after inherited handling. See [Base-part destruction logging](BASE_DESTRUCTION_LOGGING.md). The health-level callback supplies no attacker; do not correlate it with a stale cached hit or infer a successful raid from whole-object `GetHealth()`.

`EEOnDamageCalculated` is a pre-application decision hook. A result observed there may still be rejected. This patch does not override it or change its return value.

## Consumer and performance integration

The existing logger assigns event IDs, sequence numbers and boot metadata, serializes to JSON, then buffers output. The hook does not open files, flush for each hit, send HTTP requests or emit additional client RPCs. Serialization remains synchronous and should be measured under real raid load.

The queue's existing limits and dropped-event reporting still apply. This is not a lossless audit guarantee under process crashes, queue overflow or persistent disk errors.

Update any downstream event allowlist/schema to accept `basebuilding_damage`, the typed `data.zones` array, and `action.result: observed`. If records are uploaded again, use the original `eventId` for idempotency. A timestamp/source/ammo combination is not a safe duplicate key for rapid hits. This patch does not modify an external Agent, CMS, dashboard or alert rule.

If notifications become noisy, group observations separately for presentation while retaining raw records. Do not silently discard hits to reduce notifications, and do not sum copied results into an exact HP-loss total.

## Native validation

Run these checks on the actual DayZ version and mod combination, using the normal PBO build and a test server. No native test was executed while preparing this patch.

| Scenario | Expected observation |
| --- | --- |
| Build and start | Enforce modules compile; no missing symbol, duplicate override or serializer error. |
| One shot into a built fence zone | `basebuilding_damage` appears, source/ammo/zone are sensible, and `data` is an object. |
| Fast automatic fire | No 500 ms suppression; distinct callbacks receive distinct event IDs. |
| Two simultaneous attackers | Each observation uses its own source; there is no shared last-attacker state. |
| Ground grenade, mine or IED | Positive callbacks are retained with physical source/ammo; unresolved attribution has no invented Steam ID. |
| Explosive in a player's inventory | A current carrier is not automatically assigned as the attacker. |
| Multiple affected zones/objects | Positive zone entries are present; events are interpreted as callbacks, not unique explosions. |
| Final hit destroys a part | Damage is recorded when a callback is delivered; remaining object health does not gate it. |
| `source = null` or missing identity | Positive damage is retained with unknown/qualified attribution. |
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
