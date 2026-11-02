# Death-screen component compatibility

Copy init.c and CustomDeath.layout into an isolated offline ChernarusPlus mission.
Run the Stable diagnostic client with CF, the current ServerSentry PBO and a separate
profile. A disposable character dies to exercise the real native death fade.
No Agent, CMS or Discord request is sent.

The fixture uses ServerSentryDeathReportButton, overriding only its transport
boundary. Click Report suspected cheating, then Send report at each stage. Cancel first returns
to the initial button without invoking transport:

1. The normal native You are dead screen with an independent additive root.
2. A custom screen with unrelated widget names and the independent report root.
3. The production component attached to a slot supplied by the custom screen.

Duplicate clicks are disabled during transitions. The custom screen owner places
its own root above the native fade. The last stage displays a labelled local
receipt. Success logs [DeathScreenReview] Finished clicks=3; no reports sent.

All three stages passed through actual mouse input in the Stable diagnostic window
on 2 November 2026, with native fade=true at every click. This replaces the earlier
incomplete prototype check. It establishes these integration patterns, not every
third-party screen's compatibility. Production integration and opt-out settings
are documented in DEVELOPER_API.md. Only CMS persistence permits a real receipt.

On 2 November 2026 the confirmation was exercised through real mouse input in all
three integration modes. Cancel did not increment the send count; the three
confirmed clicks ended with Finished clicks=3. No CMS report was sent.
