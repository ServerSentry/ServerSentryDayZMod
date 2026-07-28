# ServerSentry Enforce Script Documentation Style

ServerSentry source documentation is written in English and follows the documentation-comment style commonly used in Bohemia script sources.

## Comment format

Use a block comment immediately before the documented declaration:

```c
/**
    \brief Returns the direction vector from point p1 to point p2.
    \param p1 \p source point.
    \param p2 \p destination point.
    \return \p Direction vector from p1 to p2.
*/
vector Direction(vector p1, vector p2)
{
    return vector.Direction(p1, p2);
}
```

Supported tags used by this project:

- `\file` identifies the documented source file.
- `\brief` gives one concise sentence describing a file, class, field, or method.
- `\param` documents every method parameter using the exact parameter name.
- `\return` documents every non-`void` return value.
- `\note` is reserved for behavior that is important but not obvious from the signature.

## Coverage rules

Every `.c` source file contains:

1. a file-level documentation block;
2. documentation for every class or enum;
3. documentation for every class field;
4. documentation for every method, including constructors and overridden DayZ callbacks;
5. `\param` entries for all parameters;
6. a `\return` entry for all non-`void` methods.

Local variables are intentionally not documented with block comments. Their names should explain their purpose, and comments inside method bodies are reserved for non-obvious behavior, compatibility constraints, or safety checks.

## Language and wording

- Documentation is always written in English.
- Use the present tense: “Returns”, “Creates”, “Registers”, “Handles”.
- Describe behavior, not syntax.
- Explain side effects such as persistence, registration, cache updates, and idempotency.
- Avoid repeating the type when the meaning is already clear.
- Keep each `\brief` sentence short enough to scan in Workbench.

## DayZ compatibility style

The documentation rules do not change Enforce Script syntax. ServerSentry also keeps the conservative source-layout rules required by the supported DayZ toolchain:

- method declarations stay on one physical line;
- method calls keep their complete argument list on one physical line;
- no fluent chains are used;
- complex arguments are assigned to typed local variables before a call;
- extension PBOs register actions through `modded class`, `override`, and `super`.

## Public API example

```c
/**
    \brief Implements a shop action that assigns or removes a player color.
*/
class MyPlayerColorAction : ServerSentryModActionHandler
{
    /**
        \brief Returns the metadata descriptor published for this live action.
        \return \p Configured action descriptor.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto descriptor = ServerSentryLiveModActionDescriptorFactory.ShopPlayerLive("shop.player.example.color", "Example: Player Color");
        return descriptor;
    }

    /**
        \brief Executes the live action using the supplied request context.
        \param ctx \p action context containing request, target, and input data.
        \param message \p receives a human-readable result or error message.
        \return \p true when the action succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        message = "Not implemented.";
        return false;
    }
}
```
