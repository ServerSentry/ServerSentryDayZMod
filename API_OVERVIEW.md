# ServerSentry Core API Overview 1.4.2

## Script modules

### `scripts/3_Game`

Contains data transfer objects, configuration, JSON helpers, logging models, input descriptors, and action request/result models. Code in this module must not depend on World or Mission classes.

### `scripts/4_World`

Contains live-action handlers, action registration, target resolution, shop fulfillment, persistence adapters, entity hooks, and runtime logging.

### `scripts/5_Mission`

Contains mission lifecycle hooks, client/server RPC handlers, support-menu UI, item export, and chat-provider registration.

## Main extension points

### `ServerSentryModActionHandler`

Base class for a live action. An integration implements:

- `GetDescriptor()` to expose action metadata and website inputs;
- `Execute()` to validate input and apply the operation.

### `ServerSentryModActionRegistry`

Stores handlers by action ID, rejects accidental duplicate IDs, keeps descriptor order deterministic, and executes the selected handler.

### `ServerSentryModActionBootstrap`

Registers built-in actions and exposes `RegisterExtensionActions()` for integration PBOs. Extensions must call `super.RegisterExtensionActions()`.

### `ServerSentryLiveModActionDescriptorFactory`

Creates base descriptors for supported target types. Configure optional metadata after the factory call by assigning fields or calling `AddInput()` as separate statements.

### `ServerSentryLiveModActionInputFactory`

Creates website input schemas for text, color, number, boolean, list, object, map, select, time, and JSON controls.

### `ServerSentryModActionContext`

Carries request identifiers, target identifiers, and action-specific JSON input. It also provides simple scalar accessors and the product-operation key.

### `ServerSentryShopFulfillment`

Wraps idempotent grant and removal operations. `BeginProductOperation()` reserves the operation and `Finish()` persists the final state.

## Recommended integration flow

1. Define a typed input DTO in `4_World` when the action has multiple fields.
2. Derive an action from `ServerSentryModActionHandler`.
3. Build the descriptor with short one-line calls and separate assignments.
4. Parse the action input with `JsonFileLoader<T>.LoadData()`.
5. Validate all values before starting fulfillment.
6. Call `ServerSentryShopFulfillment.BeginProductOperation()`.
7. Apply or remove only the effect owned by the current product.
8. Call `ServerSentryShopFulfillment.Finish()`.
9. Register the action in a `modded class ServerSentryModActionBootstrap` extension.

See `DEVELOPER_API.md` for complete examples and `DOCUMENTATION_STYLE.md` for source-comment conventions.

### Live Map base detail providers

- `ServerSentryLiveBaseDetailProvider`: filters and enriches a base marker DTO.
- `ServerSentryLiveBaseDetailRegistry`: executes providers after Vanilla rows are populated.
- `ServerSentryLiveDetails`: key-based add/replace/remove helpers; later writes override earlier values.
- `ServerSentryLiveBaseDetailBootstrap`: extension registration hook for integration PBOs.

See `DEVELOPER_API.md` for a complete FlagPole group-name and override example.

### Map markers from `3_Game`

`ServerSentryMapMarkers.Create`, `CreateCircle`, and `CreateLine` register server-side markers and return a `ServerSentryMapMarker` handle. The handle provides `SetPosition`, `SetRadius`, `SetPoints`, `AppendPoint`, `IsActive`, and `Remove`. Both classes and their shared registry live in `3_Game`; they have no World entity or file-export dependency.

`ServerSentryLiveApi` reads the same registry when building World snapshots. Existing advanced DTO-based World calls remain adapters to that registry. See the map marker examples and lifecycle rules in `DEVELOPER_API.md`.
