# Support feature flags

Use `init.c` as an isolated Stable diagnostic ChernarusPlus server mission. Copy
`WorldProbe.c` into the isolated test PBO's `scripts/4_World` and `MissionProbe.c`
into its `scripts/5_Mission`. These probes expose state for tests only and must
not be copied into the production mod. Use CF, a separate profile and port, and
do not configure an Agent to watch this test profile.

Run once with `EnableTickets=false` and `EnableSupportCases=false`, and once with
tickets enabled and support cases disabled. Live telemetry may remain disabled:
the server writes `ServerSentry/features.json` independently at mission start.
Both runs completed 12 checks with `[SupportFeatureReview] Finished failures=0`.
The fixture verifies actual module startup, lazy chat/notification allocation,
no requests or timeout state when disabled, rejecting menu creation before any
widgets load, independent tickets/refunds, binding deduplication and cleanup.
It issues no real ticket requests, notifications or refund claims.

`EnableTickets` and `EnableSupportCases` default to true, preserving older server
configs. `EnableDeathReports` remains opt-in. Clients start with every feature
disabled until the server sends the three flags in `ModFeatures`. Restart the
server after config edits and deploy the same updated mod to clients.

The CMS uses the authenticated Agent's read-only `dayz_mod_features` job with its
configured server ID/profile. Missing feature information stays unknown and is
shown as an update/availability hint. Known disabled support cases show:

> This feature is disabled by the mod you can enable it in the server config.

The backend blocks creation and release; the Agent separately rechecks before
provisioning. History and recall remain available. Disabled tickets skip Agent
request-directory processing and notification output. The normal combat/live
features continue under their own settings. Script and input definitions still
compile with the PBO; disabled support has no runtime bindings, bridge objects,
pollers, chat state or created menu widgets in the Mod.

Verification: 95 relevant Agent tests, 23 backend support-case tests, existing
frontend workflow/provisioning/item-configuration regressions, TypeScript and
production build. Browser QA used actual components with local API fixtures at
390/768/1440 widths: exact notice, disabled Save actions, enabled-state recovery,
no horizontal overflow or page errors. No real support cases were created or
released. A connected game/client/Agent/CMS deployment test remains outstanding.
