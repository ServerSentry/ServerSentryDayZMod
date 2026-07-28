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
