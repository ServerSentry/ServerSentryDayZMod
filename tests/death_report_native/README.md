# Death report data foundation

Run `init.c` as an isolated Stable diagnostic server ChernarusPlus mission with CF
and the current ServerSentry sources. Use a separate profile and port. The fixture
uses synthetic identities and local telemetry; do not point an Agent at its profile.
Success logs `[DeathReportReview] Finished failures=0`.

This step retains only the resolved suspect Steam ID and emitted kill event ID
server-side until respawn or disconnect, one latest death per Steam account, with
a 1024-account bound and mission cleanup. There is no fixed reporting timeout.
It does not keep another combat-event snapshot.
It reuses existing combat classification, explosive attribution and hitmap fields
through the regular telemetry pipeline.
`EnableDeathReports` in `$profile:ServerSentry/config.json` defaults to `false`,
including migrated configs. It also requires `EnableTelemtryLogging` and
`LogPlayerKills`. Restart the server after changing settings. Disabled reporting
allocates no report cache, captures no additional explosive-killer position and
returns no report reference; ordinary telemetry continues under its own settings.
There are no report polling loops or report files in this implementation step.
The server sends the loaded `ModFeatures` flags once when the player is ready.
The client starts disabled and resets at mission end; future report UI must use
its `IsDeathReportingEnabled()` gate before creating any reporting widgets.
The PBO's script definitions still compile as part of the mod; this flag disables
feature runtime work, not the game's script compilation.
Nonlethal hits, AI victims, other owners, invented IDs and invalidated context are
rejected. Reporting depends on player-kill telemetry being enabled. Retention in
memory is not report delivery, durable storage, or a player-facing acknowledgement.
The existing `MissionServer.OnClientRespawnEvent` clears the authenticated owner's
reference after inherited respawn handling. Disconnect clears it before inherited
handling can detach the identity. This cleanup never deletes CMS reports or
telemetry and does not change a command already obtained for submission.

For explosive player deaths the existing live-player resolver captures the known
killer's position at the hit. The explosion origin stays separate. An offline or
unresolved killer has no recorded position; the report must show that explicitly.
The fixture temporarily replaces the config singleton only in this isolated test
mission, checks disabled/default/legacy settings, then opts in for report tests
and restores the original config. It exercises native explosive-source resolution, event creation,
logging and context lookup. The connected killer's position branch still requires
a live two-player test; synthetic coordinates do not establish that integration.

The report payload is `{"reportedSteamId64":"...","killEventId":"..."}`. The Agent
adds the configured server ID from its server store, not from the game client.
`killEventId` means the emitted telemetry `eventId`, not the boot-local `hitId`.
The existing CMS unique key is `(DayZServerId, EventId)`. The CMS must check the
referenced combat kill and suspect match, derive the reporter from its victim,
and deduplicate reports by that server/kill key. Unknown attribution leaves the
suspect empty instead of borrowing a device owner's identity.

Remaining steps: submit this command through the authenticated Agent, persist a
separate CMS report linked to the combat event, and only then show the receipt confirmation.
No ticket or Discord link is involved. The death-screen button is still a separate
compatibility prototype; there is no production reporting RPC in this step.
Commands may arrive before the telemetry upload. The CMS must handle that ordering
without inventing evidence, losing the report, or creating duplicates on retries.
CMS persistence must retain the recorded actor position and explosive provenance:
the existing retained combat-detail row preserves hitmap data and victim position,
but these extra facts currently live in the raw telemetry payload and would expire
with raw-log retention. A reference to that payload alone is insufficient.

CMS requirements agreed for subsequent steps:

- A death-screen click sends the report without opening a ticket. Show only
  `Report received. The admin team will review it.` after durable CMS receipt.
- Show killer and victim positions at the death, explosive provenance, and the
  existing hitmap. Reuse `CombatHitFigure` and the existing telemetry queries.
- Reuse the authenticated self-teleport action for `Go to killer position` and
  `Go to victim position`. Require its existing server access, permissions and
  live Steam-linked admin checks. These destinations are historical positions.
- Rank reported players by distinct reporters in a rolling window, with one
  report per death and idempotent retries. Display counts and the reason for the
  priority; report volume is not a finding of cheating.
- Present a factual evidence summary alongside the admin's own assessment and review
  status. Unknown facts remain unknown; no automatic verdict or ban is implied.

## Recorded verification, 2 November 2026

Bohemia AddonBuilder packed the separate test PBO successfully. The actual Stable
diagnostic server compiled the Game/World/Mission modules and completed 40 native
checks with `[DeathReportReview] Finished failures=0`. The test server used port
2422 and its own profile; the regular development PBO was not replaced. The
fixture exercises the disconnected/unknown explosive actor paths; the connected
killer position branch and real death-screen submission remain unverified.
The latest run also verifies that native JSON serialization emits only the two
command fields, with no copied combat event or client-facing evidence payload.
It checks per-player invalidation, repeated cleanup, other-player isolation and
preserving an already obtained command. The actual server lifecycle hooks compile;
a connected player's real respawn click is not simulated by this fixture.
The feature-flag checks cover defaults, legacy JSON, explicit opt-in/disable,
config default merging, missing telemetry prerequisites, disabled-state cache
allocation and lookups, and releasing cache memory. The actual config migration
also added `EnableDeathReports: 0` to the isolated profile. The shared feature-settings
RPC compiles, but delivery to a connected client is not covered by this fixture.
