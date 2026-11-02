# Death-screen integration probe

This is an offline compatibility experiment, not the production reporting feature.
No Agent requests, CMS reports or Discord messages are sent. Its locally created
character is disposable and is killed to exercise the real native death fade.

## Recorded outcome, 2 November 2026

- Verified visually in the actual Stable diagnostic client at 1280x900: the
  additional button and its label render above the native `You are dead` fade.
- Mouse verification was blocked by a Windows scheduled-shutdown notice. The
  attempted click was rejected as belonging to the non-target PickerHost window.
  Neither the mouse click nor the custom-menu/explicit-slot scenarios passed yet.
- The system notice announced shutdown at 02:38 local time. It was not dismissed
  or cancelled by the test. The isolated probe was stopped after recording this
  result. The normal server/client and production code were not changed here.

Copy this directory into an isolated ChernarusPlus mission and run the Stable
diagnostic client with CF and ServerSentry. Use an isolated profile.

1. The native `You are dead` screen remains active. Inspect and click
   `Report suspicious death` below it.
2. A simulated third-party menu appears with unrelated widget names. The same
   report button remains an independent widget with its own event handler.
3. After clicking again, the same component is attached to a slot explicitly
   supplied by the custom menu. Click it a third time.

The log records `[DeathScreenReview] CLICK stage=1/2/3`, retains the native fade,
and ends with `[DeathScreenReview] Finished clicks=3; no reports sent`.
Mouse clicks must be verified through the actual window; invoking `OnClick`
from a script does not establish pointer/input compatibility.

The probe establishes whether a shared additive component is viable. It does
not establish compatibility with every third-party death-screen mod. In
production, a death-screen owner must be able to disable the automatic overlay
and attach the component or call the report action from its own button. Reporting
state must not rely on the names or layout of another mod's widgets.

The agreed workflow is a separate CMS report area, with Steam identification and
no Discord account requirement. The player-facing action submits the selected
server-owned death reference and shows a receipt only after CMS persistence;
it must not open a ticket form or conversation.
The server, not the client, supplies reporter identity, death time, location and
any verified combat attribution. Killer details must not be exposed by the UI.
One report per death, idempotent retries and an explicit unknown attacker avoid
duplicate reports and invented attribution. The UI button must not replace the
native fade, respawn flow or another mod's input handling.
