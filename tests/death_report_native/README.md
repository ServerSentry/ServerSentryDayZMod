# Native death-report regression

Run init.c as an isolated Stable diagnostic ChernarusPlus server mission with CF
and ServerSentry, using a separate profile and port. Synthetic identities are
used; never configure a live Agent to watch this profile.

Success logs [DeathReportReview] Finished failures=0. The 42 checks exercise
feature flags, native kill references, the two-field JSON command, ownership and
respawn/disconnect invalidation, mission cleanup, grenade attribution, unknown
killers, separate positions and hitmap fields. The production bridge publishes a
command plus ready marker and preserves that durable command when stopped.

The fixture request ID is 11111111-2222-4333-8444-555555555555, under
$profile:ServerSentry/reports/requests. An isolated cross-component check on
2 November 2026 passed this native command through the actual Agent parser/bridge,
CMS ingestion/report service and PostgreSQL migrations. It verified pending
telemetry, receipt correlation, server-derived victim, validated suspect,
historical positions and native hitmap in the resulting report DTO.

The separate death_screen_native fixture verifies production widgets through real
mouse input. Synthetic coordinates do not establish connected two-player combat.
The live RPC code compiles; that multiplayer path is not simulated here.
