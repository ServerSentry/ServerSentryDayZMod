# Server Sentry for DayZ

[![DayZ](https://img.shields.io/badge/DayZ-Server%20Mod-000000)](https://dayz.com/)
[![Language](https://img.shields.io/badge/Enforce%20Script-DayZ-orange)](https://community.bohemia.net/wiki/DayZ:Enforce_Script_Syntax)
[![Version](https://img.shields.io/badge/version-1.4.8-blue)](#versioning)

Server Sentry is a DayZ mod that exposes structured telemetry, live server-state snapshots, administrative actions, shop fulfillment, item catalog exports, and player support-case delivery through a file-based integration layer.

The mod is designed to be consumed by an external Server Sentry service or by custom DayZ integration PBOs using the included developer API.

> [!IMPORTANT]
> This repository contains the **DayZ mod source**. A packaged PBO/release build is required for normal server deployment.

## Features

- **Structured telemetry** written as newline-delimited JSON (`.jsonl`)
- **Live server snapshots** with players, vehicles, world events, bases, available actions and server FPS
- **Administrative live actions** through an inbox/outbox file bridge
- **Player actions** such as heal, teleport, spawn item, explode, voice mute and unmute
- **World actions** such as setting time/weather and spawning items at coordinates
- **Object actions** for inspection and deletion
- **Idempotent shop fulfillment** with persistent operation receipts
- **Item catalog export** for weapons, magazines, inventory items, vehicles, slots, attachments, and compatible magazines
- **Support-case delivery menu** with the default `L` key binding
- **Character backups** with per-player retention and administrator-controlled CMS restoration
- **Extensible developer API** for custom events, live markers, actions, and shop products
- **Optional integration hooks and models** for systems such as Trader, safe zones, CodeLock, breaching charges, LBmaster groups/ATM, and custom mods

## Requirements
- [Community Framework (CF)](https://steamcommunity.com/sharedfiles/filedetails/?id=1559212036)
`JM_CF_Scripts` is declared as a required addon in `config.cpp`.

CF must include `CF.FindFileEx`, which handles the DayZ 1.30 file-enumeration regression ([DZEXP-134](https://report.bistudio.com/issues/DZEXP-134)).

Some integrations are optional and only become active when the corresponding mod or compile-time symbol is available. For example, Dr. Jones Trader `TRADER`

## Installation

### Server owners

1. Build or download a packaged Server Sentry mod release.
2. Place the packaged mod in the DayZ server directory, for example:

   ```text
   @ServerSentry/
   ├── addons/
   │   └── ServerSentryDayZMod.pbo
   └── mod.cpp
   ```

3. Install and load Community Framework.
4. Add Server Sentry to the server launch parameters after CF:

   ```text
   -mod=@CF;@ServerSentry
   ```

5. Add the same client-facing mod set to the launcher or server collection when players need the support-menu UI and key binding.
6. Start the server once to generate the Server Sentry profile directories and configuration.
7. Stop the server, review `$profile:ServerSentry/config.json`, and restart it.

### Developers

Clone the repository:

```bash
git clone https://github.com/Krypton91/ServerSentryDayZMod.git
cd ServerSentryDayZMod
```

Use DayZ Tools/Addon Builder or your existing mod build pipeline to pack the project as a PBO. The source root containing `config.cpp` must become the `ServerSentryDayZMod` PBO prefix expected by the script paths.

## Configuration

On first start, the mod creates:

```text
$profile:ServerSentry/config.json
```

The configuration controls the main runtime features and individual telemetry categories.

| Setting | Purpose |
| --- | --- |
| `ConfigVersion` | Managed config schema version; currently `1`. Do not increase it manually. |
| `EnableItemExport` | Enables generation of the item catalog files. |
| `EnableTelemtryLogging` | Enables structured event/telemetry logging. The property name intentionally matches the current source schema. |
| `EnableLiveDataLogging` | Enables live snapshots and the live-action poller. |
| `LiveDataUpdateIntervalSeconds` | Controls how often the live snapshot is refreshed. |
| `LiveBaseDetailsCacheSeconds` | Reuses base/provider details for 30 seconds by default; `0` disables caching. |
| `LogAnimalHits` / `LogAnimalKills` | Controls animal combat events. |
| `LogZombieHits` / `LogZombieKills` | Controls infected combat events. |
| `LogVehicleEngineStart` / `LogVehicleEngineStop` | Controls vehicle engine events. |
| `LogEnterVehicle` / `LogExitVehicle` | Controls vehicle occupancy events. |
| `LogVehicleDeletion` | Controls vehicle deletion events. |
| `LogInventoryMovement` | Controls player inventory movement events. |
| `LogPlayerHits` / `LogPlayerKills` | Controls player combat events. |
| `LogBaseBuilding` | Controls base-building events. |
| `LogBaseDamage` | Records only hits that ruin a base entity or damage zone; defaults to `false`. Requires `EnableTelemtryLogging`. |
| `LogActionDismantlePart` | Controls dismantle events. |
| `LogActionDestroyPart` | Controls destroy-part events. |

The mod migrates existing configurations on startup. Files without `ConfigVersion` are version `0`; the current schema is version `1`. Missing settings are written with their current defaults, including fields inside nested sections. Existing values, explicit `false` or `0`, and unknown properties are preserved. For example, an absent `LiveBaseDetailsCacheSeconds` is added as `30`, while an existing `0` remains `0`.

Missing fields are repaired even when `ConfigVersion` is already current. An unchanged configuration is not rewritten. Before replacing a changed file, the mod verifies a temporary copy and preserves the previous contents as `config.json.before-v<previous-version>` (for example, `config.json.before-v0`). A later repair of the same version refreshes that backup. Invalid JSON and unsupported newer versions are not rewritten. Startup logs report migration success or the reason it could not be saved.

### Character backups

`CharBackupConfig` in `$profile:ServerSentry/config.json` controls character snapshots:

```json
"CharBackupConfig": {
  "EnableCharacterBackup": true,
  "MinCharacterLiveTimeSeconds": 3600,
  "MinItemsInInventory": 1,
  "MaxBackupFiles": 5,
  "RestoreCooldownSeconds": 60
}
```

Backups are captured **only when a player dies**, before vanilla death cleanup removes the character's persistence managers. `EnableCharacterBackup` must be enabled, the authenticated character must meet `MinCharacterLiveTimeSeconds` and `MinItemsInInventory`, and `MaxBackupFiles` must be positive. There are no interval, disconnect or shutdown captures. A character's death is processed once. Nested contents and held items count toward `MinItemsInInventory`; the player itself does not. The feature supports administrator recovery after suspected cheating; it does not automatically decide whether a death involved a cheater. Capture and restore work independently of telemetry/live logging.

The versioned config migration also populates a missing or null `CharBackupConfig` and adds missing fields in an existing backup section. The default minimum character lifetime is **3600 seconds**. `BackupIntervalSeconds` is retained as an ignored legacy field; even an existing positive value never schedules a capture. A backup directory may therefore exist before any snapshot is eligible. Startup logs show the effective policy; skipped captures and failed writes report their reason.

`MaxBackupFiles` is a limit **per Steam ID**. Oldest snapshots are removed after a successful capture and when the mission starts, including after reducing the limit. Zero (or a negative value) removes existing committed snapshots at startup and disables captures/restores. Disabling `EnableCharacterBackup` preserves files subject to the configured retention limit.

For a disposable local test, set `MinCharacterLiveTimeSeconds` to `0` and optionally `RestoreCooldownSeconds` to `5`. Restart, connect with at least one item, and kill the test character. Respawn with the same survivor class before restoring. In the CMS, select that Game Server, open the player profile, choose **Restore**, select a backup and confirm. The player must be alive and idle on foot. Use the updated CMS, Agent and server mod together.

New death snapshots (version 3) contain the death position/orientation and restore the player's health/blood/shock pools to full, without restraint, unconsciousness or active bleeding. They also contain persistent player statistics (including energy, water, temperature, blood type and specialty), stamina, lifespan data, diseases/modifiers, bleeding, stomach contents, damage permission/god mode, restraint/unconsciousness flags, quickbar bindings, and the complete carried inventory tree. Items retain their exact attachment/cargo/hand locations, damage, scripted persistence, quantities, liquids, wetness, food state, energy, internal electrical connections and ammunition (including mixed magazine rounds and internal magazines). Mod state participates when its `OnStoreSave`/`OnStoreLoad` callbacks persist it, including CF mod storage.

The complete snapshot is first loaded into a temporary server character. Original items are retained until the replacement inventory and player state commit. A `.items.json` sidecar records persistent and mission-local identities alongside each new snapshot. Preserve it together with the `.bin` file. On a restore failure, original items are moved back and player state is loaded from a temporary rollback snapshot. An incomplete rollback keeps `restore.rollback` and logs its path. Further restores for this player refuse to overwrite that recovery file; originals that could not be moved back are retained on the ground or in the staging container; the server log identifies incomplete recovery. Temporary/rollback files are separate from committed `.bin` snapshots.

Restoration currently requires a living player with the **same survivor class**, the same world and DayZ persistence version, and compatible installed mods. Respawn with a different survivor model is rejected before replacing inventory. Spent cartridge cases become empty chambers because the public native API has no safe setter for fired cases. The service does not rewind external databases such as bank balances, groups or bases, or arbitrary mod values that are not saved through entity persistence. It does not revive an existing corpse; restore a living respawn of the same survivor class instead. For new backups, identifiable loaded original items are moved back from corpses, world containers or other online players. Matching inventory subtrees are transferred intact, including weapons with attached magazines; only changed parent/slot relationships are moved. Gameplay-blocked original-item moves are rejected before replacing the recipient inventory. Existing originals keep their current ammunition, quantities, condition and scripted state; they are not refilled from the snapshot. Missing items are recreated. Unrelated cargo inside reclaimed containers is retained; if it blocks a saved location, the transaction rolls back instead of deleting it. Legacy version-2 snapshots lack item identities and cannot reconcile originals.

This is best-effort duplication reduction, not a global persistence-database transaction. Items in offline inventories, unloaded containers, or storage controlled by another mod may be unavailable to the runtime lookup and therefore duplicated when replaced. No background world scan is performed. Only IDs referenced by retained backups are indexed; the index is initialized from sidecars and updated as those entities load or are deleted.

Each backup is usable once. A `.restore-pending` marker is written before mutation and a `.restored` marker after success. Incomplete rollback or a crash leaves the backup locked for manual investigation; do not remove these guards without inspecting the server state. A successful rollback clears the pending marker. Retention prunes the snapshot, identity sidecar and restore markers together. Keep a complete backup directory when moving server profiles. Access remains restricted to authorized CMS administrators.

The dedicated `character.backups` and `character.restore` actions use the Agent file bridge, with only the `character_backups` scope. They are not advertised for generic Live Map or shop execution. Listing reads at most ten native headers and their identity sidecars per page; it does not deserialize inventory script streams. An online living character receives a two-minute, one-use permit bound to the current player instance and compatible backups on that page. Restoration consumes the permit before any mutation; disconnect/respawn, expiry, another list request or a server restart invalidates it. `RestoreSelected(player, backupId, message)` accepts only generated filenames beneath that authenticated player's directory. The original world/save-version/survivor checks and rollback remain in effect.

## Generated files and directories

Server Sentry stores runtime data below the DayZ profile directory:

```text
$profile:ServerSentry/
├── config.json
├── items.json
├── items_count.json
├── events/
│   └── *.jsonl
├── live/
│   ├── state.json
│   ├── bases-<session>-<revision>.json
│   ├── events-<session>-<revision>.json
│   └── actions-<session>-<revision>.json
├── actions/
│   ├── inbox/
│   │   └── *.json
│   └── outbox/
│       └── *.result.json
├── fulfillment/
│   └── shop/
│       └── actions/
├── SupportCases/
│   └── <steam-id>.json
├── CharacterBackups/
│   └── <steam-id>/
│       └── <UTC timestamp>_<sequence>.bin
└── voice-mutes/
    └── data.json
```

### Telemetry

Telemetry events are buffered and written as JSON Lines under:

```text
$profile:ServerSentry/events
```

Every line is an independent JSON event. Events include schema information, a logical server ID, a boot ID, a sequence number, timestamps, actors, targets, actions, and event-specific data.

### Base damage

Enable `LogBaseDamage` and `EnableTelemtryLogging` to record only hits that change a base entity or one of its damage zones to ruined. This covers `BaseBuildingBase` and `BuildingBase`, including a destroyed fence section while the rest of the fence remains intact. Ordinary hits and further hits on already ruined zones produce no event. A qualifying `basebuilding_damage` event retains the damage type, source class and positive named-zone damage values. Explosive attribution uses the same resolver as combat hits and kills.

See [Base damage logging](docs/BASE_DAMAGE_LOGGING.md) for the payload contract, integration steps, attribution limits and native test matrix.

Building, dismantling and destroy-action logs remain independent of this final-hit filter. Direct scripted health changes without a hit callback do not produce a base-damage event.

### Explosive attribution

Fragmentation grenades, landmines, claymores, plastic explosives, IED charges and tripwire grenades retain separate placement, arming, live-release, remote/damage-detonation and triggering-victim identities. The common resolver uses explicit detonation provenance first, then a known live release, then the arming identity. A player stepping on a mine is not assigned their own death merely because they triggered it. Unknown actors remain unknown; current explosive carriers and nearby players are never used as guesses.

Attribution appears in `combat.explosive` for hits/kills and `data.explosive` for base damage. Each physical detonation has a shared UUID and blast position; each activated IED charge has its own device/detonation identity and a parent-device link. Player IDs and names are copied at the server operation, so the responsible player can disconnect before the explosion. `weapon.distance` for explosion hits measures blast origin to victim; it does not claim the offline player's current position.

Device roles persist in the isolated `ServerSentry` CF ModStorage stream (`CfgMods.ServerSentry.storageVersion = 1`). Vanilla persistence fields are unchanged. Existing devices without these records remain unattributed until a new observed arming/release operation. IED and tripwire roles are transferred before charges detach. Unsupported mod activation paths and later poison/bleeding deaths require a separate integration rather than invented attribution. See [Explosive attribution](docs/EXPLOSIVE_ATTRIBUTION.md) for the integration contract and native verification steps.

### Live Map export performance

Snapshot requests, including legacy marker upserts and prefix removal, are combined before building any DTOs. The exporter waits 250 ms for related updates and leaves at least one second between completed writes. A clear-and-repopulate operation therefore exports the final marker set once, rather than writing the empty and partially populated sets. Regular polling still exports current measurements; no full-state fingerprint is built.

`LiveDataUpdateIntervalSeconds` defaults to 10 and is clamped to at least one second. `LiveBaseDetailsCacheSeconds` defaults to 30, accepts 0-300, and can be set to `0` to rebuild base details on every export. Cached provider details may lag until the next refresh/export. Position or orientation changes, Vanilla flag manipulation, attachment changes, provider/action registration, base removal, and mission shutdown invalidate the relevant entries.

Integrations that change ownership or other provider output can call `ServerSentryLiveApi.InvalidateBaseDetails(entity)` or omit the argument to invalidate all entries. Providers are applied after Vanilla rows whenever an entry is rebuilt, preserving their overrides. Snapshot consumers should treat returned base DTOs as read-only.

The schema-four file transport separates frequently changing players, vehicles and measurements from bases, events and action descriptors. `state.json` is a small complete frame with `sessionId`, `sequence`, and `basesRevision`/`eventsRevision`/`actionsRevision`. Each revision references a complete section file with the same session and revision. Slow sections are written only when their serialized contents change. Base serialization is also skipped while all cached base DTOs are still current.

The first export writes all sections, verifies their contents, then publishes the manifest. Later exports retain the current and previous committed section generations. The Agent validates every reference and rereads the manifest before accepting a frame; it retries missing, partial or inconsistent reads. Skipping frames is safe because referenced sections are complete states, including explicit empty collections. Native file replacement is not atomic. The Agent conservatively removes old-session section files after acknowledgement; the Mod performs no directory search.

Deploy this Mod with the coordinated Agent/CMS update. The Agent computes updates and explicit deletions against its last acknowledged CMS state. The CMS commits the revision and changed entity rows together in `dayz_live_state` and `dayz_live_entities`, and the browser requests changes since its applied revision. Session changes or unavailable baselines trigger a complete resynchronization. Mod timestamps remain separate from receipt timestamps, so a retry cannot make stale game data appear fresh.

### Inventory attachments

With `LogInventoryMovement` enabled, `item_move` events also cover attaching and detaching items. `action.name` is `item_attach` or `item_detach` for these operations. Existing pickup, drop, and storage actions remain in use for transfers involving player equipment slots.

The `data.from` and `data.to` locations include the immediate parent class/name, its session-local `parentObjectId`, and the slot ID, name, and display name. Attachments on carried weapons or clothing retain `type: "attachment"`; equipment slots directly on a player use `type: "player_slot"`. The target identifies the object receiving or losing the attachment.

Player attribution still comes from the old or new inventory owner. With the default player-only filter, moves between world objects without a player inventory owner are omitted rather than attributed to a nearby player.

### Vanilla digital code locks

On DayZ 1.30 and later, `DigitalCodeLock` (including `DigitalCodeLock_Bunker`) emits `codelock_event` events with `mod` and `data.sourceMod` set to `DayZ`. These events follow `EnableTelemtryLogging` and the existing code-lock logging default.

| `action.name` | Recorded operation |
| --- | --- |
| `access` | Keypad interaction, including rejected interaction data or lock-side validation. Opening the keypad is separate from a checked code request. |
| `lock` | The player locks an unlocked digital lock. |
| `unlock` | A checked code unlocks the lock, fails to unlock it, or the player releases it from the physical inside. |
| `set_code` | Initial code setup, with the result determined from the resulting lock state. |
| `reset_code` | A player resets the configured code. |

Payloads contain the actor identity/position, lock and parent object information, resulting lock state, access method, and outcome (`success`, `failed`, `denied`, or `ignored`). PIN values are never included. Failed requests are reported from the observed server state. Merely opening the keypad does not count as a successful unlock. Installing or removing the lock or its battery is covered by the inventory attachment events.

### Live state

The current live manifest is written to:

```text
$profile:ServerSentry/live/state.json
```

The manifest and its referenced section files together contain:

- online players;
- vehicles;
- world markers and events;
- tracked base objects;
- registered administrative actions;
- current, minimum, and maximum server FPS;
- generation time and schema metadata.

### Item catalog

When item export is enabled, Server Sentry generates:

```text
$profile:ServerSentry/items.json
$profile:ServerSentry/items_count.json
```

The exporter scans public entries from `CfgVehicles`, `CfgWeapons`, and `CfgMagazines`. It skips a full export when the relevant configuration state has not changed.

To force a new export, create this marker file before the next server start:

```text
$profile:ServerSentry/force_item_export
```

The marker is removed after a successful export.

## Live actions

When live data is enabled, the server checks the action inbox once per second:

```text
$profile:ServerSentry/actions/inbox/*.json
```

The poller uses `CF.FindFileEx` to enumerate the existing inbox files, including the DayZ 1.30 filesystem workaround. The request format and the writing service do not need to change.

After execution, the request file is removed and a result is written to:

```text
$profile:ServerSentry/actions/outbox/<requestId>.result.json
```

Built-in action IDs include:

| Target | Action ID | Description |
| --- | --- | --- |
| Player | `player.heal` | Restores a player's health state. |
| Player | `player.teleport` | Teleports a player. |
| Player | `player.spawn_item` | Spawns one or more items for a player. |
| Player | `player.explode` | Triggers the configured explosion action on a player. |
| Player | `player.voice.mute` | Applies a persistent voice mute. |
| Player | `player.voice.unmute` | Removes a voice mute. |
| Object | `object.inspect` | Returns information about a world object. |
| Object | `object.delete` | Deletes a selected world object. |
| Server | `server.time.set` | Changes the world time. |
| Server | `server.weather.set` | Changes weather values. |
| World | `world.item.spawn_at` | Spawns an item at coordinates. |
| Shop | `shop.player.bank.deposit` | Deposits currency through a compatible bank integration. |
| Shop | `shop.spawn_bundle` | Delivers an item bundle with optional attachments. |

> [!WARNING]
> The action inbox can trigger powerful administrative operations. Only the DayZ server process and a trusted Server Sentry service should have write access to `$profile:ServerSentry/actions/inbox`.

## Support cases

Support cases are loaded from:

```text
$profile:ServerSentry/SupportCases/<steam-id>.json
```

Players can open the support menu with the default `L` key. After confirmation, the server attempts to place the configured items in the player's inventory and falls back to spawning them on the ground when necessary. The case file is deleted after processing.

The key binding is declared as `UAToggleSupportMenu` in `Inputs.xml` and can be changed through the normal DayZ input settings.

## Developer API

The repository includes two API documents:

- [`API_OVERVIEW.md`](API_OVERVIEW.md) — modules and primary extension points
- [`DEVELOPER_API.md`](DEVELOPER_API.md) — complete Enforce Script examples and compatibility rules

### Script modules

| Module | Responsibility |
| --- | --- |
| `scripts/3_Game` | DTOs, configuration, JSON helpers, logging models, and public event APIs. |
| `scripts/4_World` | Runtime hooks, entity access, live actions, shop fulfillment, and live snapshots. |
| `scripts/5_Mission` | Mission lifecycle, RPC handling, support UI, chat capture, and item export. |

Lower modules must not depend on classes that are only available in later script modules.

### Emit a custom event

Custom integrations can use `ServerSentryApi` without modifying the logger internals:

```c
string dataJson = "{\"value\":42}";
ServerSentryApi.EmitCustom(
    "MyMod",
    "my_event",
    "integration",
    "example",
    "My custom event",
    dataJson,
    player,
    "info"
);
```

Keep every Enforce Script method call on one line. This is a project compatibility rule for the supported DayZ toolchain.

### Register a custom action

Create a handler derived from `ServerSentryModActionHandler`, then register it through the bootstrap extension point:

```c
modded class ServerSentryModActionBootstrap
{
    override void RegisterExtensionActions()
    {
        super.RegisterExtensionActions();
        ServerSentryModActionRegistry.Register(new MyCustomAction());
    }
}
```

Always call `super.RegisterExtensionActions()` so multiple integration PBOs remain compatible in the `modded class` inheritance chain.

### Shop fulfillment

Shop actions should use `ServerSentryShopFulfillment.BeginProductOperation()` and `ServerSentryShopFulfillment.Finish()` to make grants and removals idempotent. The request's `fulfillmentKey` is used when available; otherwise the request ID becomes the operation key.

## Project structure

```text
ServerSentryDayZMod/
├── config.cpp
├── Inputs.xml
├── gui/
├── scripts/
│   ├── 3_Game/
│   │   ├── Database/
│   │   ├── Logging/
│   │   └── models/
│   ├── 4_World/
│   │   ├── Actions/
│   │   ├── Logging/
│   │   └── hooks/
│   └── 5_Mission/
│       ├── GUI/
│       ├── ItemExport/
│       └── Logging/
├── API_OVERVIEW.md
├── DEVELOPER_API.md
└── DOCUMENTATION_STYLE.md
```

## Troubleshooting

### The mod does not load

- Confirm that Community Framework is installed and loaded before Server Sentry.
- Check the server script log for missing addon or script-module errors.
- Confirm that the packed PBO uses the expected `ServerSentryDayZMod` prefix.
- Confirm that the server and clients use compatible mod versions when the client UI is enabled.

### No event files are created

- Check `EnableTelemtryLogging` in `config.json`.
- Confirm that the DayZ server process can write to the profile directory.
- Search `script.log` for messages beginning with `[ServerSentry]`.
- Trigger a known event such as a player login or logout.

### No live snapshot is created

- Check `EnableLiveDataLogging`.
- Verify that `$profile:ServerSentry/live` can be created.
- Check `LiveDataUpdateIntervalSeconds` for an unexpectedly large value.

### Actions remain in the inbox

- Confirm that live data logging is enabled, because it starts the action poller.
- Confirm that the loaded CF version provides `CF.FindFileEx` and that the Server Sentry PBO was rebuilt with the updated poller.
- Validate that the request contains a non-empty `actionId`.
- Check `script.log` for `[ServerSentry][LiveAction]` messages.
- Ensure the request target and input JSON match the selected action descriptor.

### Item export does not run again

The exporter caches the relevant DayZ configuration counts. Create `$profile:ServerSentry/force_item_export` to force the next export.

## Contributing

1. Fork the repository.
2. Create a focused feature branch.
3. Test on a dedicated server and review `script.log` for serialization or runtime errors.
4. Open a pull request with a clear description of the behavior and test coverage.

## Versioning

The current mod version in `config.cpp` is **1.4.8**.

When publishing a release, update the version in `config.cpp` and keep the API documentation version aligned with the implementation.

## Security

- Restrict write access to action and support-case directories.
- Validate all external JSON before placing it in the server profile.
- Do not expose the DayZ profile directory through a public web server.
- Keep fulfillment receipts persistent to prevent duplicate product delivery.
- Back up the profile directory before changing schemas or deploying major updates.

## License

This repository currently does not include a `LICENSE` file. Until a license is added, no open-source usage rights should be assumed beyond rights explicitly granted by the copyright holder.

## Author

Created by **Krypton91**.
