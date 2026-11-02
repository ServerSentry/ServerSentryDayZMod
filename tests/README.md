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


Combat source persistence checks (requires native DayZ compilation/runtime):

- Fire two distinct rifles of the same class. Verify each hit/kill carries the
  actual `weapon.persistentId` and matching `item.persistentId`, and that inventory
  move, hit and kill observations join only that exact item on the same server.
- Repeat a melee hit with a persistent tool, then bare hands, AI and infected
  attacks. Physical tools may have an ID; bare hands/environment must stay empty.
- Trigger a grenade, placed charge and chain explosion. Capture source data before
  inherited hit callbacks remove the device. Never substitute deviceId,
  detonationId, runtime GetID(), the controller or the player's currently held gun.
- When a ranged source is only the player (or projectile cannot establish a gun),
  verify no identity is guessed from a weapon switched into hands later.
- Verify all-zero native persistence blocks remain an empty ID. Repeat after
  restart and compare item history across inventory moves and combat records.
- Retest damage, kill classification, deduplication and base ruined-zone logging;
  compare callback overhead with the same hit rate. This change adds no world scan.

Support-menu navigation regression (native offline client):

Use `menu_navigation_native/init.c` as an offline ChernarusPlus mission with CF
and ServerSentry loaded. It checks first-click and repeated section navigation,
widget focus after returning to Tickets, closing the entire support stack,
preserving the pause menu's input exclusion, and releasing exclusions owned by
a direct hotkey entry. It logs `[MenuNavigationReview] Finished failures=0` on
success and leaves an empty reimbursement open for mouse-only verification.
Check the Tickets sidebar button once, close with X, reopen Support from the
pause menu, and click Continue. The cursor must remain available while the pause
menu is visible. This offline fixture does not send tickets or claim any items.

Ticket conversation regression (native offline client):

Use `ticket_chat_native/init.c` as an offline ChernarusPlus mission with CF and
ServerSentry loaded. Its fake transport exercises the actual conversation menu
and chat state without contacting Discord. It checks unconfirmed delivery with
editable text, automatic receipt lookup using the original request ID, retaining
a newer draft after late success, success while the menu is closed, reopening,
ordinary successful sends, reply-status updates with unchanged rendered messages,
immediate Waiting for support after a confirmed own reply, and compatibility with
an older backend that omits reply status. Success logs
`[TicketChatReview] Finished failures=0`. Run the matching backend and Agent builds
to receive the current reply status in the regular game session. The ticket list
and conversation use the last public reply: creator replies (including in-game
bot embeds) wait for support; other replies wait for the player. Opening/control
embeds do not change the reply status. Reading or sending a conversation does not
fetch the reply status of every other ticket during ownership checks.

Ticket notification regression (native offline client):

Use `ticket_notifications_native/init.c` with CF and ServerSentry in an isolated
offline ChernarusPlus mission. It verifies the native ten-second duration, delayed
activation when the native notification queue is full, duplicate suppression,
expiry/consumption, suppression for an already-open conversation, fresh/expired
inbox files, and the actual shortcut handler opening the exact conversation with
the Tickets sidebar root. Success logs `[TicketNotificationsReview] Finished
failures=0`. The fixture never sends Discord messages. Backend/Agent tests cover
the configured guild, ownership/link, online player/server/Agent routing, stale
messages, atomic file output and disabled/unknown server rejection. A live Discord
gateway-to-game check requires the matching backend and Agent builds running.

Direct-opening performance checks in the same native fixtures verify that the
notification starts a conversation read before the shortcut is pressed, the
hidden ticket list issues no request (including after normal UI updates), and
returning to the list still loads it normally. The chat fixture checks coalescing
same-ticket reads, immediately starting a queued different-ticket read, preserving
queued-send priority, and refreshing a notified ticket after an older in-flight
read. These are deterministic native request-order checks, not a measured live
Discord latency benchmark.

Ticket creation/options cache checks:

`ticket_options_native/init.c` captures the actual client request method's RPC
boundary in an offline ChernarusPlus mission. It checks immediate reuse of valid
types, lightweight form queries, fresh list queries, forced refresh, expiry,
unlinked/error rejection, correlation IDs, and native JSON request flags. Success
logs `[TicketOptionsReview] Finished failures=0`; it makes no backend or Discord
requests. Agent `DayZTicketLinkBridgeTests` verify per-player/server cache keys,
the remaining lifetime, absolute expiry, the 256-entry bound, forced-refresh
failure without stale fallback, uncached unlinked accounts, legacy request
defaults and fresh ticket-list queries despite cached types. Sending a ticket
continues through the existing CMS validation.

Support feature flags:

See `support_features_native/README.md` for the isolated module-startup and
disabled-runtime checks. Offline UI fixtures explicitly apply server feature
settings because they do not receive a real server's readiness handshake.
