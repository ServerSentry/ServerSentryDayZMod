# Native validation

Build the PBO with the supported DayZ toolchain and use a local test server before distributing changes. Validate the Mod and integrations in DayZ itself, including Enforce Script compilation, native inventory behavior, serialization and disk latency.

- Live export regression: request prefix removal plus two marker upserts in one callback. Confirm one final `Wrote snapshot` line and one complete output, rather than three writes. Repeat while regular polling is due. A second write must not occur within one second of the previous completion. Confirm requests stop on mission shutdown and resume after the next start.
- Base cache: count detail-provider calls across three 10-second polls with a 30-second TTL. Check expiry, a moved/rotated/deleted flag, raising/lowering and attachment changes, provider/action registration, and `InvalidateBaseDetails(entity)`. Set the TTL to zero and check every export rebuilds details. Verify a custom ownership change refreshes after explicit invalidation or cache expiry.
- Section transport: verify schema-four `state.json` references complete matching session/revision files. Leave all bases unchanged and confirm their file is reused across polls; change one flag or remove all flags and confirm the next manifest references the changed or empty complete section. Stop the Agent across several Mod frames, resume it, then restart each component independently. Verify CMS and browser converge to the current state without stale objects. Exercise lost upload acknowledgements, partial files and a missing older section while the Agent is reading. Do not treat `CopyFile` as atomic replacement.
- Production timing: compare the same player/base population and the same safezone refresh period before/after. Record output sizes, writes per burst, and the independent one-second timer gaps. Screenshots alone cannot reproduce the reported 83% overlap, 270 KB payload, or 137 ms profiler sample. Native JSON serialization and disk IO remain synchronous and need measurement on the actual server.

- Markers: create a point, circle and line from a separate integration PBO; verify creation, updates and removal in the events section referenced by `live/state.json`. Check replacement IDs and mission cleanup. Declare `ServerSentryScripts` as the integration's addon dependency.
- Inventory: compare a connected test character's equipment, held-item attachments and nested cargo with its snapshot. Add/remove disposable items; check full or incompatible destinations, moved items, disconnect/respawn, capture expiry and server restart.
- Performance: verify simulation interval samples and loaded/ground item counts during pickup, placement and deletion. These counters describe loaded entities, not a persistence-database inventory of the entire map.
- Combat: compare damage-zone and component selection names in hit records with known targets. Keep the development-only damage-zone exporter hooks disabled on production servers.
- Character backups: use a disposable server profile and connected test characters for the checks below. Compare saved and restored native inventory and character state, then reconnect and restart to verify persistence.

Character-backup checks requiring a connected client:

1. Use a disposable profile with `MinCharacterLiveTimeSeconds=0`, `MaxBackupFiles=2`. Kill and respawn each of two test players more than twice; verify each player's directory contains only their two newest `.bin` files. Reduce the limit and restart; check pruning without needing another login. Test zero retention separately.
2. Capture nested cargo, an item in hands, attachments, quickbar shortcuts, a partial liquid bottle, food, a battery, mixed magazines and multi-muzzle weapons. Change inventory, location/orientation, health/blood/shock, water/energy, temperature, disease, bleeding, broken legs and stomach state. Select that exact backup through the CMS Restore dialog while idle. Compare with the saved state, then reconnect/restart and compare again.
3. Confirm no snapshot before the age/item thresholds, while disabled, while alive, on disconnect, after waiting, or on clean shutdown. Confirm exactly one snapshot on an eligible death, including non-combat deaths. Confirm backups operate with telemetry and live logging disabled. Confirm the O key has no restore binding or RPC. Verify CMS permission/server-scope checks, selected backup identity, cooldown and expired or duplicate requests.
4. Truncate a snapshot, change its persistence version/world/survivor class, or remove a saved mod item. Verify restore fails before touching the live inventory. Exercise an incompatible live inventory move and verify rollback, including the retained rollback file on an unrecoverable move. Check client/server logs for script errors and ensure temporary character trees are deleted.
5. Restore a living respawn with the same survivor class; verify an existing corpse is not revived or deleted. Verify a different survivor class is rejected. For newly recreated missing weapons, verify spent cases become empty chambers while live ammunition and weapon jams remain consistent. Reclaimed original weapons must retain their current contents. Verify the dedicated CMS action works with live telemetry disabled when character backups are enabled. Reuse a consumed token, change character, reopen the list and restart the server; old permits must be refused without changing inventory.

6. Move backed-up items to another online player's hands, nested world storage and a corpse. Restore and check the exact original instances, including changed quantities/ammunition. Add unrelated cargo that occupies a saved location and check rollback returns each item to its former owner. Verify unrelated cargo is never silently deleted.
7. Restart with retained snapshots and sidecars; verify persistent identities reconnect as world items and player inventories load. Verify consumed backups remain unavailable after restart. Simulate a pending restore marker and confirm it fails closed. Test offline/unloaded inventory separately: replacement is allowed by policy and can leave a duplicate when the old item loads later.

API contracts and examples are documented in [DEVELOPER_API.md](../DEVELOPER_API.md).

Map marker icon runtime checks (CMS and Agent with icon support):

- Compile and load the Mod; confirm living/dead players export `UserRound`/`Skull`.
- Check ordinary cars, trucks, helicopters, motorcycles and boats export their
  respective names. Check base and custom-event icon names in the live JSON.
- Inspect the same entity in the live map and log/AI context map. A `truck` marker
  must use the bundled SVG; a `Sailboat`/`Plane` marker must use Lucide when there
  is no same-name SVG. An unknown name must use a pin in both views.
- Change only a marker icon while preserving identity/position; verify the next
  incremental update refreshes the glyph in both views. Retest after reconnect.
- Verify an older Mod payload without icons keeps its semantic type defaults.
