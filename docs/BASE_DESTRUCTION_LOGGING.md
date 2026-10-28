# Base-part destruction logging

## Scope and status

The `basebuilding_part_destroyed` event observes a built base part that becomes unbuilt during an engine health-level transition to `GameConstants.STATE_RUINED`.

The hook is implemented in `modded class Construction`. Vanilla `Construction` is the component used by `BaseBuildingBase` objects and inherits the relevant ruined-part handling from `ConstructionBase`. Keeping the wrapper on `Construction` avoids extending the feature to unrelated constructed entities.

The source was reviewed against the supplied ServerSentry project and Bohemia's script declarations. No DayZ/Enforce compiler or running game server was available during preparation. The native checks below remain required before distributing a compiled PBO.

## Configuration

Set these properties in the existing `$profile:ServerSentry/config.json`, with the server stopped:

```json
{
  "EnableTelemtryLogging": true,
  "LogBaseDestruction": true,
  "LogBaseDamage": false
}
```

This is a fragment, not a replacement for the complete configuration. `LogBaseDestruction` defaults to `false`; existing migration logic adds missing fields without discarding administrator values. The configuration version remains unchanged because existing field semantics are unchanged.

Set `LogBaseDamage` to `true` separately if individual hit observations are also required. Neither event depends on `LogActionDestroyPart`, `LogActionDismantlePart` or `LogBaseBuilding`.

## Hook ordering

The wrapper performs these steps:

1. For server-side enabled logging, require a named zone, an old level other than ruined and a new level equal to ruined.
2. Lowercase a copy of the zone to match the vanilla construction lookup.
3. Resolve the parent and script part. Only a currently built part is eligible.
4. Copy the target identity, position, part name and callback levels into a pending DTO. No entity or construction-part reference is retained in the DTO.
5. Call `super.OnHealthLevelChanged(oldLevel, newLevel, zone)` exactly once, regardless of whether logging is enabled or an event was prepared.
6. Inspect the locally captured script part. Emit only when the part still exists and now reports `IsBuilt() == false`.

The wrapper never calls `DestroyPartServer`, changes health or overrides a damage decision. Inherited code owns destruction, dependent parts, synchronization, physics and attachment handling.

A per-construction array tracks only part names currently inside the synchronous wrapper. It prevents reentrant observation of the same part and is cleared when that invocation returns. There is no timestamp-based deduplication, persistent last-attacker cache or delayed callback. A rebuilt part can be observed again when it is destroyed later.

After the inherited callback, the hook does not read the parent entity again. Target metadata was captured beforehand, and the postcondition uses only the local script-part reference. If that reference becomes unavailable or the part remains built, the pending event is discarded.

## Event contract

| Envelope field | Value |
| --- | --- |
| `eventType` | `basebuilding_part_destroyed` |
| `category` | `basebuilding` |
| `action.name` | `basebuilding_part_destroyed` |
| `action.result` | `observed` |
| `severity` | `info` |
| `actor.type` / `actor.entityKind` | `unknown` |
| `target.type` / `target.entityKind` | `object` / `basebuilding` |
| `position` / `targetPosition` | Parent world position captured before inherited handling |

| `data` field | Meaning |
| --- | --- |
| `sourceHook` | `Construction.OnHealthLevelChanged` |
| `targetId` | Runtime object ID; not persistent base ownership or identity |
| `partName` | Canonical name returned by the captured construction part |
| `damageZone` | Original callback zone, preserving its case |
| `oldLevel` / `newLevel` | Engine health-level categories, not HP values |
| `wasBuiltBefore` | `true` for an emitted event |
| `isBuiltAfter` | `false` for an emitted event |
| `attackerResolution` | `unavailable_in_health_callback` |

The pending DTO initializes `isBuiltAfter` to `true`. Only the post-super observation changes it to `false` and permits emission.

The callback contains no damage source, attacker identity, ammo, damage amount or hit identifier. Those fields are not reconstructed from a previous hit. A direct scripted health change can cause the same transition, so this event alone does not prove firearm damage, an explosion, a malicious raid or the success of an entire raid.

## Interaction with existing events

| Event | Observation |
| --- | --- |
| `basebuilding_damage` | Positive engine damage result in a hit callback |
| `basebuilding_part_destroyed` | Confirmed part-state outcome during a ruined health transition |
| `basebuilding_action` | Existing player construction/dismantle/destroy action |

A final hit may legitimately create both a damage observation and a destruction outcome when both switches are enabled. Do not count both as independent attacks.

Vanilla explicit destroy actions and connected-part removal normally unbuild a part before setting its remaining zone health to zero. Those later health callbacks fail the prebuilt gate and do not create another destruction event through this path. Existing action logs remain independent.

The hook therefore records directly observed ruined-part transitions. It is not a complete inventory of every dependent part removed during a collapse. Dismantling, scripted removals bypassing this callback, non-`BaseBuildingBase` construction objects and mods that defer removal until after the callback returns need separate integration if they must be covered.

## Serialization and consumers

The `3_Game` DTO/event classes contain only copied values. World-entity access remains in the `4_World` factory and hook. `ServerSentryLogger.Emit` explicitly dispatches the new event to `JsonFileLoader<ServerSentryBaseDestructionEvent>` before using the existing event queue and JSONL writer.

Output remains under `$profile:ServerSentry/events`. Existing event IDs, sequence, server/boot metadata and queue limits apply. The new hook performs no per-event file open, network request or forced flush.

Downstream schemas and event allowlists must accept `basebuilding_part_destroyed`, `action.result: observed` and its typed `data` object. The complete mod archive does not contain an update for a separate Agent/CMS project.

## Native verification

| Case | Expected outcome |
| --- | --- |
| Build/start with the actual mod set | Enforce compilation and JSON serialization succeed |
| Partial damage, still built | No destruction event; an optional damage event may exist |
| Fatal health transition of a built zone | One confirmed destruction observation for that part |
| Repeated ruined callback on an unbuilt part | No further destruction observation |
| Build the same part again, then ruin it | A new destruction observation is allowed |
| Super/integration leaves the part built | Pending observation is discarded |
| Empty zone or zone not mapped to a construction part | No destruction observation; inherited handling still runs |
| Explicit dismantle/destroy action | Existing action behavior remains; no duplicate record from an already-unbuilt part's health update |
| Connected-part collapse | Directly ruined part is observed; dependent removals are not promised as extra records |
| Missing player or source | Event remains valid with explicit unknown attribution |
| Telemetry disabled or destruction switch disabled | No events through this path |
| Damage logging disabled, destruction enabled | Destruction events still work independently |
| Synchronous reentry for the same part | No second pending observation for that part |
| Parent removed by another mod | No post-super target access; native compatibility still requires testing |
| Agent/CMS ingestion | New event and typed payload are accepted and displayed |

## Official source references

- [Construction: inherited health handling, parent access and destruction ordering](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/classes/basebuilding/construction.c).
- [ConstructionBase: ruined-health part lookup](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/classes/constructionbase.c).
- [BaseBuildingBase: part state synchronization and deferred construction removal](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/4_world/entities/itembase/basebuildingbase.c).

The referenced `main` branch is moving. Validate against the DayZ version and mods actually deployed on the server.
