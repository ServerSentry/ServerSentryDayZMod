# Native validation

Build the PBO with the supported DayZ toolchain and use a local test server before distributing changes. Source checks and tests with stand-in game APIs do not replace compiling Enforce Script or checking native inventory behavior.

- Markers: create a point, circle and line from a separate integration PBO; verify creation, updates and removal in `live/state.json`. Check replacement IDs and mission cleanup. Declare `ServerSentryScripts` as the integration's addon dependency.
- Inventory: compare a connected test character's equipment, held-item attachments and nested cargo with its snapshot. Add/remove disposable items; check full or incompatible destinations, moved items, disconnect/respawn, capture expiry and server restart.
- Performance: verify simulation interval samples and loaded/ground item counts during pickup, placement and deletion. These counters describe loaded entities, not a persistence-database inventory of the entire map.
- Combat: compare damage-zone and component selection names in hit records with known targets. Keep the development-only damage-zone exporter hooks disabled on production servers.

API contracts and examples are documented in [DEVELOPER_API.md](../DEVELOPER_API.md).
