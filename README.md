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
| `EnableItemExport` | Enables generation of the item catalog files. |
| `EnableTelemtryLogging` | Enables structured event/telemetry logging. The property name intentionally matches the current source schema. |
| `EnableLiveDataLogging` | Enables live snapshots and the live-action poller. |
| `LiveDataUpdateIntervalSeconds` | Controls how often the live snapshot is refreshed. |
| `LogAnimalHits` / `LogAnimalKills` | Controls animal combat events. |
| `LogZombieHits` / `LogZombieKills` | Controls infected combat events. |
| `LogVehicleEngineStart` / `LogVehicleEngineStop` | Controls vehicle engine events. |
| `LogEnterVehicle` / `LogExitVehicle` | Controls vehicle occupancy events. |
| `LogVehicleDeletion` | Controls vehicle deletion events. |
| `LogInventoryMovement` | Controls player inventory movement events. |
| `LogPlayerHits` / `LogPlayerKills` | Controls player combat events. |
| `LogBaseBuilding` | Controls base-building events. |
| `LogActionDismantlePart` | Controls dismantle events. |
| `LogActionDestroyPart` | Controls destroy-part events. |

The generated file should be treated as the authoritative schema for the installed version. Newly added settings are initialized by the mod when older configurations are loaded.

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
│   └── state.json
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
└── voice-mutes/
    └── data.json
```

### Telemetry

Telemetry events are buffered and written as JSON Lines under:

```text
$profile:ServerSentry/events
```

Every line is an independent JSON event. Events include schema information, a logical server ID, a boot ID, a sequence number, timestamps, actors, targets, actions, and event-specific data.

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

The current live snapshot is written to:

```text
$profile:ServerSentry/live/state.json
```

The snapshot can include:

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
