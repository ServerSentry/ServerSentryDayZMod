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
