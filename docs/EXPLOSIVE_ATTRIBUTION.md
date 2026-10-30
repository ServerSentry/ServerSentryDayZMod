# Explosive attribution

## Scope

Combat hits, combat kills and final base-damage hits use `ServerSentryExplosiveResolver`. Supported vanilla paths cover live fragmentation-grenade releases, landmines, claymores, plastic explosives, remote/timed IED charges and tripwire grenades. No tests are added to the Mod. The implementation has been checked against the local DayZ and installed CF script sources; native compilation and runtime verification remain required after packing.

## Ownership and event contract

`combat.explosive` and `data.explosive` use the same `ServerSentryExplosiveAttribution` model:

| Field | Meaning |
| --- | --- |
| `deviceId` | UUID attached to the physical device and retained through persistence. |
| `parentDeviceId` | IED or tripwire controller that supplied the charge's activation context. |
| `detonationId` | Runtime UUID shared by victims and base hits of the same physical detonation, including multi-ammo claymore blasts. |
| `parentDetonationId` | Upstream explosion UUID for a captured chain reaction. |
| `sourceClassName`, `sourcePosition` | Actual physical blast source; the position is not the current location of a player. |
| `placedBy` | Player who placed the device. Informational; placement alone does not award kills. |
| `armedBy` | Player responsible for the current arming operation. |
| `releasedBy`, `releaseObserved` | Player releasing a live grenade, and whether such a release was observed even without an identity. A missing releasing identity is not replaced with an earlier arming actor. |
| `detonatedBy` | Explicit remote operator, damaging shooter or responsible actor of an observed chain reaction. |
| `triggeredBy` | Player stepping onto a mine or crossing a tripwire. Never used to award that player's death. |
| `activation` | Fuse, trap, timer, remote, damage or unknown activation path. |
| `resolution` | `detonated_by`, `released_by`, `armed_by` or `unresolved`. |

Each identity contains copied actor fields, the establishing operation and its UTC time. It contains no player/entity reference. The selected identity is also copied into the existing top-level event actor so current hit/kill statistics can use it without an online-player lookup. Explosion classification uses `DamageType.EXPLOSION`. Direct player and firing-weapon sources can resolve without a stored device record; unknown physical sources stay unknown.

Priority is explicit detonation, then a known live release, then a known arming identity. Unknown remote operators or external damaging causes block a fallback to an old owner. The player who merely triggers the trap remains a separate role. A damaging actor is committed only when that damage actually ruins/kills the explosive; earlier non-lethal hits do not replace the arming or throwing actor.

For explosion combat records, `weapon.distance` is blast-to-victim distance. The resolver does not fabricate `actorPosition` from the blast origin or from a saved action position.

## Hook lifecycle

- `ActionUnpin.OnExecute` records successful server-side pin removal; `ActionPin` clears the previous cycle only after successful repinning.
- `Grenade_Base.EEItemLocationChanged` captures an unpinned hands-to-ground release before inventory ownership disappears. This covers a throw and a drop; it deliberately does not label every release as a throw.
- `ExplosivesBase.OnPlacementComplete` records the supplied server actor before subclass code can arm the device. `OnArmed` starts a fresh cycle. Trap placement/deployment and `StartActivate(player)` cover landmine/tripwire paths; persistence calls with a null player do not erase an existing identity.
- `ActionTriggerRemotely.OnExecuteServer` scopes an authenticated operator to that action. Device hooks accept it only for their paired remote. An arbitrary current holder or an automatic remote activation does not supply that identity.
- IED activation and tripwire deactivation copy controller provenance to the attached charges before native code detaches them or schedules later activation. The actual charge keeps its own physical UUID.
- A device's native `EEKilled(killer)` supplies the actual killing source before inherited code detonates it. Earlier non-lethal or rejected hits never set a last-attacker cache. Known fuse and paired-remote operations have a synchronous activation scope so a null/self killing source preserves the established release/arming roles. Other unknown killing sources do not silently fall back to an earlier owner. Plastic explosives preserve their native non-exploding behavior on bullet damage and use their explicit activation adapter.
- `ItemBase.Explode` freezes provenance before native explosion damage is dispatched. Victim hooks copy the source into their hit request before invoking inherited hit behavior. Hits and kills therefore use the same source snapshot.

All inherited callbacks run once and inherited damage decisions are preserved. Attribution does not invoke explosions, damage, arming or inventory moves itself. There is no global explosion proximity search, player enumeration, polling timer or file write for each hit. Attachment enumeration is limited to the controller being activated. Ordinary loot never allocates an attribution object.

## Persistence

The already-required Community Framework provides an isolated `ServerSentry` stream with storage version 1. Only tracked devices write entries. Optional identities are stored as small typed entries rather than one potentially truncated native JSON string. Runtime explosion UUIDs, frozen blast snapshots and active fuse/remote call scopes are not restored as live detonations.

No fields are appended to vanilla `OnStoreSave`/`OnStoreLoad` streams. Old items without ServerSentry data remain usable and unattributed. Missing, incomplete or unsupported attribution is discarded without deleting the item or guessing an owner. Existing armed mines cannot acquire a historical owner retroactively. A later observed rearming operation establishes a new relationship.

## Integrations and limits

Mods derived from the supported vanilla devices inherit this behavior when they preserve the relevant callbacks. An additional ItemBase explosive can expose `IsExplosive()` and explicitly supply server-verified identities through `ServerSentryExplosiveState`, `ServerSentryArmExplosive` and `ServerSentrySetExplosiveState`. These are server-side helpers, not client RPCs. Unsupported activation methods remain unresolved.

The actor selection is an explicit logging policy, not proof of intent. A remote operator or observed damaging actor takes precedence over the earlier placer. For an automatically triggered trap, the recorded arming actor is used while the triggering victim is retained separately. Keep these roles visible when presenting moderation evidence.

Poison/contaminated-area deaths, later bleeding deaths and script-only health mutations without a matching damage source require their own causal integration. This change does not assign an earlier grenade hit to an arbitrary later death.

## Native verification

Use a disposable server profile with the freshly packed Mod and its required CF version:

1. Player A unpins/releases a fragmentation grenade; damage and a direct kill resolve to A with `released_by`. Surviving hits and the lethal hit use the existing combat classification and event deduplication.
2. Repeat while A disconnects before detonation. Saved identity must still resolve. Let B pick up and release the live grenade: the later release belongs to B. Repinning/rearming must not reuse an old release.
3. A arms a mine, disconnects and restarts the server; B triggers it. A is the responsible actor, B is `triggeredBy`. A mine predating this feature stays unresolved until rearmed.
4. A places/arms a paired claymore or plastic explosive; B operates its remote. B is `detonatedBy`, A remains in the earlier roles. Both claymore ammo effects share the same detonation UUID. Deactivation/rearming starts a new cycle.
5. Test remote and timed IEDs with several charges. Each physical charge has its own UUID and carries the controller's actor roles and parent UUID after detaching. Repeat with a tripwire grenade and a different triggering victim.
6. Shoot a live grenade to cause a detonation; the final damaging shooter takes precedence. A non-lethal earlier shot must not take credit for a later normal fuse explosion. Repeat with a chain reaction and check the upstream detonation link.
7. Unknown sources and explosives detonated in someone else's inventory must not use that carrier as an inferred attacker. No fabricated IDs or nearby-player search.
8. Cause non-final and final base hits with the same explosives. Ordinary hits stay suppressed; only a newly ruined object/zone emits base damage, using the same provenance as combat.
9. Inspect script/RPT for compilation or persistence errors. Confirm normal explosions, arming, damage and inventory transitions behave as before and compare logging overhead under representative load.
