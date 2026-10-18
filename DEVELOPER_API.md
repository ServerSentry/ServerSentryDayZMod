# ServerSentry Core Developer API 1.4.2

This API targets DayZ Enforce Script. It deliberately uses a conservative source style:

- no fluent method chains;
- no function or method call whose argument list continues on another line;
- no nested factory call inside another factory call;
- long descriptions and help texts are assigned to local variables first;
- every returned DTO is stored in a typed local variable before it is changed.

The one-line call rule is a ServerSentry compatibility rule for the supported DayZ toolchain. Bohemia documents Enforce functions as fixed, typed parameter lists and documents `override`, objects, arrays, references, and `modded class`; it does not require a C# builder style.

## Source documentation

All ServerSentry Enforce Script declarations are documented in English with Bohemia-style documentation blocks:

```c
/**
    \brief Executes the live action using the supplied request context.
    \param ctx \p action context containing request, target, and input data.
    \param message \p receives a human-readable result or error message.
    \return \p true when the action succeeds; otherwise false.
*/
```

See `DOCUMENTATION_STYLE.md` for the complete convention and `API_OVERVIEW.md` for the module and extension-point overview.

## Shop action descriptor

```c
class MyColorAction : ServerSentryModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        string description = "Sets or removes a player color.";
        ServerSentryLiveModActionDescriptorDto descriptor = ServerSentryLiveModActionDescriptorFactory.ShopPlayerLive("shop.player.my_mod.color", "My Mod: Player Color");
        descriptor.icon = "Palette";
        descriptor.description = description;

        string colorHelp = "Choose the product color.";
        ServerSentryLiveModActionInputDto colorInput = ServerSentryLiveModActionInputFactory.Color("hexColor", "Color");
        colorInput.required = true;
        colorInput.placeholder = "#FFFFFF";
        colorInput.defaultValue = "#FFFFFF";
        colorInput.helpText = colorHelp;
        descriptor.AddInput(colorInput);

        string removeHelp = "Removes this exact product effect instead of applying it.";
        ServerSentryLiveModActionInputDto removeInput = ServerSentryLiveModActionInputFactory.RemoveProduct();
        removeInput.label = "Remove product effect";
        removeInput.helpText = removeHelp;
        descriptor.AddInput(removeInput);

        return descriptor;
    }
}
```

Do not chain factory results and do not split any method-call argument list across multiple lines. Store long strings in local variables first.

For vehicle and FlagPole actions, the bundled samples now build the descriptor with explicit field assignments.

## Addon registration

```c
modded class ServerSentryModActionBootstrap
{
    override void RegisterExtensionActions()
    {
        super.RegisterExtensionActions();
        ServerSentryModActionRegistry.Register(new MyColorAction());
    }
}
```

Use `modded class` for the extension point and keep `super.RegisterExtensionActions()` so other loaded integration PBOs remain in the inheritance chain. Duplicate action IDs are rejected unless an intentional replacement passes `true` to `ServerSentryModActionRegistry.Register` as the second argument.

## Live Map FlagPole details

ServerSentry creates Vanilla FlagPole details first and then runs registered detail providers. Details use a stable, case-insensitive key. Calling `UpsertBaseDetail` with an existing key replaces its label and value in place; a new key appends a new row. Providers run in registration order, so the last provider writing a key wins.

Vanilla base keys are:

- `type`
- `className`
- `objectId`
- `position`
- `health`
- `radius`
- `flagHeightPercent`
- `lifetimeRemaining`
- `lifetimeMax`

Create a provider in the integration mod:

```c
class MyFlagPoleDetails: ServerSentryLiveBaseDetailProvider
{
    override bool Supports(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        return TerritoryFlag.Cast(entity) != null;
    }

    override void ApplyDetails(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        string groupName = ResolveMyGroupName(entity);
        ServerSentryLiveDetails.UpsertBaseDetail(dto, "groupName", "Group", groupName);

        // Same key replaces the Vanilla value.
        ServerSentryLiveDetails.UpsertBaseDetail(dto, "flagHeightPercent", "Flag status", "Protected by My Mod");

        // A Vanilla row can also be removed.
        ServerSentryLiveDetails.RemoveBaseDetail(dto, "lifetimeMax");
    }
}
```

Register it through the bootstrap hook:

```c
modded class ServerSentryLiveBaseDetailBootstrap
{
    override void RegisterExtensionProviders()
    {
        super.RegisterExtensionProviders();
        ServerSentryLiveBaseDetailRegistry.Register(new MyFlagPoleDetails());
    }
}
```

The provider receives the live `EntityAI` and the already populated `ServerSentryLiveBaseDto`, so it can filter by class, read data from its own modded entity classes, add new rows, replace Vanilla rows, or remove rows. Keep `super.RegisterExtensionProviders()` so multiple integration PBOs remain compatible.

## Live Map context-menu action

Actions whose descriptor uses the target type `contextMenu` are shown when an administrator right-clicks the Live Map. The request contains the mouse-selected world coordinate in `ctx.position`. Because the browser map does not know the terrain height, X and Z are authoritative and the supplied Y value is normally `0`. Use `ctx.GetSurfacePosition()` or extend `ServerSentryContextMenuModActionHandler` to resolve the DayZ terrain height.

```c
class MyAirdropAction : ServerSentryContextMenuModActionHandler
{
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        string description = "Spawns an airdrop at the map position selected by an administrator.";
        ServerSentryLiveModActionDescriptorDto descriptor = ServerSentryLiveModActionDescriptorFactory.ForContextMenu("world.my_mod.airdrop", "Spawn Airdrop", "Plane", description);
        descriptor.AddScope("live");
        descriptor.SetDangerous("Spawn an airdrop at the selected map position?");
        return descriptor;
    }

    override bool ExecuteAtPosition(ServerSentryModActionContext ctx, vector targetPosition, out string message)
    {
        // targetPosition already uses GetGame().SurfaceY(ctx.position.x, ctx.position.z).
        // Spawn your airdrop implementation here.
        message = "Airdrop action received for " + targetPosition.ToString() + ".";
        return true;
    }
}
```

The frontend also includes `position`, `x`, `y`, and `z` in `ctx.inputJson`. Action input descriptors named `x`, `y`, or `z` are automatically prefilled, but handlers should normally use `ctx.position` as the canonical map target.

## Simple input access

```c
bool removeProduct = ctx.IsRemoveProduct();
string value = ctx.GetString("name", "default");
int amount = ctx.GetInt("amount", 0);
float ratio = ctx.GetFloat("ratio", 0.0);
bool enabled = ctx.GetBool("enabled", false);
```

## Typed JSON input

Use a dedicated DTO for an action with multiple fields:

```c
class MyActionInput
{
    string hexColor;
    bool removeProduct;
}
```

Load it with DayZ's JSON loader:

```c
MyActionInput input;
string inputError;
if (!JsonFileLoader<MyActionInput>.LoadData(ctx.inputJson, input, inputError))
{
    message = "Invalid action input JSON: " + inputError;
    return false;
}
```

## Color parsing

```c
ServerSentryRgbColor color;
if (!ServerSentryLiveActionInput.TryParseHexColorValue(input.hexColor, color, message))
{
    return false;
}
```

The parsed result exposes `red`, `green`, `blue`, `hex`, and `ToARGB()`.

## Idempotent fulfillment

```c
ServerSentryShopFulfillmentOperation fulfillment;
int beginStatus = ServerSentryShopFulfillment.BeginProductOperation(ctx, input.removeProduct, "My product operation", fulfillment, message);
if (beginStatus == ServerSentryShopFulfillment.BEGIN_ALREADY_FULFILLED)
{
    return true;
}
if (beginStatus != ServerSentryShopFulfillment.BEGIN_READY)
{
    return false;
}

string actionMessage;
bool actionSucceeded = ApplyProduct(ctx, input.removeProduct, actionMessage);
return ServerSentryShopFulfillment.Finish(fulfillment, actionSucceeded, actionMessage, message);
```

`fulfillmentKey` is a top-level field of `ServerSentryLiveActionRequestDto`. When empty, `requestId` is used. A removal operation uses the same base key with the `:remove` suffix, so grant and removal are independently idempotent.

## Script-module placement

- `scripts/3_Game`: DTOs and helpers that do not require World entities.
- `scripts/4_World`: actions, entity resolution, persistence adapters, and LBmaster runtime integration.
- `scripts/5_Mission`: mission hooks and chat capture.

Keep lower script modules independent from classes that are only available in later modules.

## Official Bohemia references

- DayZ Enforce Script Syntax: https://community.bohemia.net/wiki/DayZ:Enforce_Script_Syntax
- DayZ Modding Basics: https://community.bohemia.net/wiki/DayZ:Modding_Basics
- DayZ Modding Structure: https://community.bohemia.net/wiki/DayZ:Modding_Structure
- DayZ Workbench Script Debugging: https://community.bohemia.net/wiki/DayZ:Workbench_Script_Debugging

### Bundled airdrop context hook

The core mod now registers `ServerSentryAirdropContextAction` with the action ID
`world.airdrop.spawn`. This makes **Spawn Airdrop** visible in the Live Map
right-click menu immediately and verifies the complete frontend/backend/mod
transport. Its default implementation only logs and confirms the selected
terrain position; it does not spawn an airdrop.

Put the actual airdrop call into:

```c
ServerSentryDayZMod/scripts/4_World/Actions/Global/ServerSentryAirdropContextAction.c
```

or override it from an integration PBO:

```c
modded class ServerSentryAirdropContextAction
{
    override bool ExecuteAtPosition(ServerSentryModActionContext ctx, vector targetPosition, out string message)
    {
        // Your airdrop implementation here.
        message = "Airdrop spawned at " + targetPosition.ToString() + ".";
        return true;
    }
}
```


## Map markers from 3_Game

The public API is `ServerSentryMapMarkers`. Its factories, marker handles, validation, and registry all live in **3_Game** and can also be used from 4_World and 5_Mission. Integrations use the factories and handles without constructing export DTOs.

Your integration PBO must depend on `ServerSentryScripts` in `CfgPatches.requiredAddons[]`. Calls require a server game instance. Creation can happen before `ServerSentryLiveApi.Start()`; registered markers are included once live export starts. Marker calls themselves perform no file IO, world queries, or callback scheduling. The existing live-export configuration and Agent/CMS pipeline still control delivery to the admin map.

### Point and circle

Creation registers the marker automatically. Store class members as `ref ServerSentryMapMarker` and check creation for null:

```c
vector dropPosition = "5000 0 6000";
ServerSentryMapMarker drop = ServerSentryMapMarkers.Create("Supply cache", "Package", dropPosition);
ServerSentryMapMarker area = ServerSentryMapMarkers.CreateCircle("Drop search area", "Target", dropPosition, 350);

if (drop)
{
    vector nextPosition = "5200 0 6100";
    drop.SetPosition(nextPosition);
}
if (area)
{
    area.SetRadius(500);
}
```

`Create(name, icon, position, radius = 0, id = "")` creates a point at radius zero and a circle at a positive radius. `CreateCircle(name, icon, position, radius, id = "")` requires a positive radius. Negative, non-finite, or greater-than-100,000 radii are rejected. Coordinates are finite DayZ X/Y/Z values within +/-1,000,000; the map uses X/Z. No terrain height lookup is needed, so Y may be supplied as zero for a static map position.

`SetPosition` preserves the radius. `SetRadius(0)` converts a circle to a point. Both methods return false for a line, invalid input, a client call, or an inactive handle. Invalid updates preserve the existing marker.

Icon names are passed through to the CMS, for example `MapPin`, `Package`, `Plane`, or `Route`. An empty icon defaults to `MapPin`. Rendering of individual icons depends on the CMS.

### Route and observed trail

```c
TVectorArray routePoints = new TVectorArray();
vector entry = "1000 600 8500";
vector waypoint = "5000 650 6500";
vector exit = "9500 600 8500";
routePoints.Insert(entry);
routePoints.Insert(waypoint);
routePoints.Insert(exit);

ServerSentryMapMarker route = ServerSentryMapMarkers.CreateLine("Airdrop route", "Plane", routePoints, "myairdrop:flight42:route");
if (route)
{
    // Replace the route atomically; vectors are copied, not retained from this array.
    route.SetPoints(routePoints);

    // For an observed trail, append actual samples on your own bounded callback.
    vector nextSample = "9800 620 8800";
    route.AppendPoint(nextSample);
}
```

Lines use the existing `shape="polyline"` payload, with the endpoint icon at the last position. They require 2-256 points. `SetPoints` validates all positions before replacing the old geometry. `AppendPoint` ignores identical adjacent samples and retains the newest 256 points. A planned route endpoint does not automatically track an aircraft; supply observed positions yourself or maintain a separate moving point.

### Identity and cleanup

Every factory accepts an optional final `id`. Use a mod/event prefix for explicit IDs. Recreating an explicit ID replaces its registration and invalidates the old handle. Generated IDs avoid collisions with current registrations. The shared registry accepts at most 512 entries, including legacy World markers; replacing an existing ID does not consume another slot. Names are limited to 200 characters, icons to 80, and IDs to 240.

```c
if (drop)
{
    bool removed = drop.Remove();
    // Further updates on this handle return false.
}

// Remove all markers belonging to a completed flight or integration.
ServerSentryMapMarkers.ClearByPrefix("myairdrop:flight42:");
```

A marker stays registered until explicitly removed, replaced, or cleared at mission finish. Dropping a handle alone does not remove the registration. Stop mod-owned update callbacks during your own cleanup. Markers are not persisted across a process restart and there is no automatic entity tracking or TTL.

Changes are exported on the configured snapshot cadence; multiple updates between snapshots coalesce. Avoid per-frame updates. Point/circle/polygon transport remains unchanged; the CMS must support `polyline` to render routes.

### Existing World integrations

`ServerSentryLiveApi.CreateMarker`, `CreatePolygonMarker`, `SetMarkerPolygon`, `AddMarkerPoint`, detail/action helpers, and DTO-based `UpsertMarker`/`RemoveMarker` remain available in 4_World and 5_Mission. Registration and removal use the same Game registry. The legacy immediate-write behavior is retained; deferred methods still use the configured export cadence. Removing or replacing a marker through the legacy API also invalidates any public handle for that registration.



## Player inventory actions

The inventory bridge provides three player-targeted actions with scope `inventory`:

| Action | Input | Result |
| --- | --- | --- |
| `inventory.snapshot` | Exact player SteamID in the request target | Current inventory and available destinations in `dataJson` |
| `inventory.add` | Capture ID, destination ID, item classname, placement version and reason | One item created at the requested destination, or a refusal |
| `inventory.remove` | Capture ID, item ID and reason | The selected item removed, or a refusal |

The CMS checks server access, inventory permissions and catalog eligibility before dispatching through the Agent's action inbox/outbox. Inventory actions are not exposed in the generic action picker or shop fulfillment.

### Snapshot format

The action result envelope contains a JSON-encoded `dataJson` string. The current inventory format is version 2:

| Field | Meaning |
| --- | --- |
| `version` | `2` for exact destination support |
| `steamId` | Requested SteamID64 |
| `online` | Whether a connected player was found |
| `snapshotId` | Capture ID, matching the snapshot request ID for an online player |
| `capturedAtUtc` | UTC capture timestamp |
| `truncated` | Whether the capture exceeded its item or destination limit |
| `items` | Captured item instances |
| `destinations` | Available hands, attachment-slot and cargo destinations |

Each item includes `id`, `parentId`, `destinationId`, `className`, `displayName`, `quantity`, `healthPercent`, `childCount` and `location`. A negative quantity means that quantity is not applicable. `parentId` identifies the immediate captured parent item; an empty value means the player. Item IDs are valid only within their capture.

Each destination includes `id`, `parentId`, `kind` (`hands`, `attachment` or `cargo`), `label` and `slotName`. Use the returned destination ID for placement. Labels and slot names are display metadata, not identifiers. A dedicated server may return untranslated stringtable keys as labels.

DayZ's JSON serializer writes boolean fields as numeric `0` or `1`. Consumers must handle this representation. An offline result means the requested player was not found; an absent or failed response does not establish that the player is offline. Version 1 snapshots lack exact destinations and cannot authorize version 2 additions.

### Adding and removing items

An add action supplies the following fields in its action input JSON:

```json
{
  "snapshotId": "<capture UUID>",
  "placementVersion": 2,
  "destinationId": "<destination ID from that capture>",
  "className": "WorkingGloves_Beige",
  "reason": "Replace missing equipment"
}
```

For removal, supply `snapshotId`, `itemId` and `reason`. The outer action envelope still requires its own request ID and exact player target.

A capture is valid for two minutes and the same player entity. It is consumed before a mutation is attempted, including a refused mutation. Request a new snapshot after each attempt. Never reuse an item or destination ID with another capture or automatically retry a mutation whose outcome is unknown.

Placement uses only the selected hands, attachment slot or container. A full, occupied or incompatible destination is refused; there is no ground or alternate-container fallback. Removing a nonempty container or an item with attachments is refused. Ownership changes, respawn, disconnect and expired captures also invalidate an operation.

The mod retains at most 128 player captures, with up to 512 items and 2,048 destinations per snapshot. Incomplete captures cannot authorize changes. Captures exist only in memory and are invalidated by a mod restart. CMS, Agent and mod must support the same inventory protocol; offline database estimates are read-only.

See [tests/README.md](tests/README.md) for local regression commands and native runtime checks.

## Runtime damage-zone diagnostics

Development-only diagnostics are disabled by default: the Reset/Observe calls in MissionServer.c and ServerSentryCombatFactory.c are commented out. Keep these hooks disabled in production; normal combat hit-zone/component logging remains active. To repeat a local diagnostic, explicitly re-enable those hooks, repack and reload the Mod. When enabled, the server exports effective damage-zone configuration when a player connects/reconnects/respawns, or a new combat-target class is encountered. Files are written under `$profile:ServerSentry/diagnostics/damage-zones/<ClassName>.json`. The exporter reads existing entities; it does not spawn or damage them and exports no player identities or positions.

Each file contains the game build, UTC time, game-reported Mod names/versions, native `GetDamageZones` results and the corresponding resolved `CfgVehicles ... DamageSystem DamageZones ... componentNames`. The reported Mod list does not guarantee addon load order. Zone/component configuration describes damage attribution, not collision polygon boundaries or the exact callback from a specific impact.

Work is bounded to one attempt per encountered class and 128 classes per mission, with at most 64 zones and 64 components per zone. `truncated` marks omitted zone/component entries. Existing class files are replaced on the next mission's first observation; old files from other classes are retained. Repeated hits on an already observed class perform only the class/cache lookup. Inspect both a class diagnostic and its normal combat records when validating finer body-part attribution.

Updated scripts must be packed and loaded by the game before these exports can be produced. API entry point for an existing server entity: `ServerSentryDamageZoneExporter.Observe(entity)` in `4_World` or `5_Mission` code.