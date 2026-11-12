# ServerSentry Workbench

Use **Start-Workbench.cmd**. This setup uses Stable DayZ / Workbench 1.30.164200 and the Dabs Workbench tools. Client, server and build settings are managed inside Script Editor; there are no separate BAT launch configurations.

| Shortcut | Workbench command |
| --- | --- |
| F8 | Close this project's diagnostic instances, then Build Mod using Bohemia Addon Builder. |
| F6 | Restart the local Stable server, then start a diagnostic client that connects automatically. Selects the server debugger. |
| Shift+F6 | Launch Server and select the server debugger. |
| Ctrl+F6 | Launch Client and select the client debugger. |
| Ctrl+Shift+F10 | Close this project's diagnostic client/server. |
| Ctrl+Shift+F6 | Configure Project: launch paths, mods, profiles, port and arguments. |
| Ctrl+F8 | Configure Build Game: packer and build arguments. |

F8 closes diagnostic instances whose profiles are inside this project's `.workbench` directory before replacing the PBO. Launch again with F6 after building. F6 starts a real multiplayer session using DayZDiag with the mod and file patching; it does not load an offline mission. F6 restarts only this project's diagnostic instances and defaults to server debugging. To debug client-side UI/scripts instead, choose **Debug → Debug Client**; choose **Debug → Debug Server** to switch back. Only one target is selected at a time. Debug Server uses port 1002; Debug Client uses 1001. F9 toggles breakpoints, F5 continues, F11 steps into and F10 steps over. Workbench may request authorization for its local scripts' file/process operations; handle those dialogs manually.

For layouts, open **@ServerSentrySources → ServerSentryDayZMod → gui** in the main Resource Browser. The support menus use stock DayZ widgets/fonts with shared button states and scrollable rows. Dabs/COT are not UI runtime dependencies. The mod's existing CF dependency remains.

F6 must be reserved for the launch plugin. In **Workbench → Options → Shortcuts**, clear the native **Script Editor → Console Execute** binding if it is also F6. This conflict was cleared in the local Workbench preferences; the console remains available through its Run button.

## Files and output

- `workbench/project.cfg`: mods and PBO prefix used by the Dabs tools.
- `workbench/launch.bin` and `build.bin`: saved Workbench dialog settings.
- `workbench/addon-include.txt`: script, config, layout, input and texture file types copied by Addon Builder. Builds use `-packonly` and the `ServerSentryDayZMod` prefix.
- `workbench/serverDZ.cfg`: local server configuration; copied to `server.cfg`. Keep `allowFilePatching = 1`.
- `.workbench/@ServerSentry/addons`: development PBO; `.workbench/mods/@ServerSentryDayZMod` points to this mod.
- `.workbench/@ServerSentrySources`: unpacked editor wrapper; its scripts/gui junctions edit the repository directly and allow repacking while Workbench is open.
- `.workbench/profiles/ServerSentryDayZMod/client` and `/server`: game profiles and logs with the default launch settings.
- `.workbench/mpmissions/ServerSentryDayZMod.ChernarusPlus`: isolated local mission/persistence.
- `.workbench/vendor/DabsFramework`: unchanged upstream checkout/license, pinned to `245190266b4e72bb0519fbfdd68bbed71c1edc1f`.
- `.workbench/dabs-tools` and `vanilla-stable/scripts`: generated compatibility copies for the editor. They are excluded from the gameplay PBO.

`Start-Dev.ps1` prepares/opens Workbench, stages game files for the build plugin, or stops this project's diagnostic processes. `Start-NativeGame.ps1` launches the plugin-generated arguments from the game's installation directory. The launcher uses the installed native Workbench project, an unpacked mod wrapper and one isolated editor-script copy; it does not generate a second project or editor mod. Generated Enforce scripts use UTF-8 without a BOM for Windows PowerShell 5 compatibility.

Workbench starts with its installed `Bin/Workbench` directory as the working directory so it can find the native `dayz.gproj`. The explicit `-repository` argument keeps mod-specific launch/build settings in this repository's `workbench` folder. Startup plugins run through Resource Manager with `-plugin=ClassName`; Initialize ServerSentry opens Script Editor.

The ESC Support button and hotkey request the player's support state from the server. An existing case opens reimbursements; otherwise My tickets opens. The reimbursement menu also has a My tickets button. Responses use request tokens, a timeout and parent-menu checks. Empty cases cannot be claimed. Ticket creation is enabled after Discord linking and selection of an available ticket type. The configured Discord membership, role and open-ticket limits remain authoritative. Item delivery still uses the existing server implementation; the UI does not claim a delivery acknowledgement that the server does not send.

Opening My tickets or the ticket form requests the Discord account-link status through the game server and Agent. The server derives the Steam ID from the RPC sender; the Agent derives the server ID from its configuration. Files are exchanged under the server profile's `ServerSentry/tickets/link-status` directory. A `.ready` marker publishes each complete request; the Agent publishes responses by renaming a temporary file. Timeouts and backend errors return `success = false`, distinct from an unlinked account. For linked accounts, the same response includes a nested `ticketList` with its own success flag and error message, so a list failure does not discard valid account-link status or ticket types. The backend only returns open tickets belonging to the linked Discord user and configured Discord server; current Discord membership is required. The client result is available through `ServerSentryClientManager.Get().GetTicketLinkStatus()`; `IsCheckingTicketLink()` reports a pending request, and `RequestTicketLinkStatus()` starts a fresh check.

My tickets shows each ticket's type, reference and support status. Selecting a row opens its summary with the UTC creation date; Back returns to the list. Refresh reloads the list, including while viewing a summary. A ticket that is no longer open returns to the list. New ticket opens the existing form; Resume request restores an unconfirmed submission. Returning from the form refreshes the list. Unlinked players see Link account and can refresh after linking on the website. The form retains Link account / Check again and the available ticket types. Conversation history and sending replies remain follow-up steps.

Discord linking and ticket configuration are separate requirements. The backend must have an enabled ticket panel with at least one ticket type allowed by the player's Discord roles. If no type is returned, New ticket stays disabled and the list explains that no ticket types are available. After changing the backend configuration, use Refresh; relinking the account is unnecessary.

Ticket creation uses `ServerSentry/tickets/create/requests` and `/responses`. Request filenames include the server-derived Steam ID and the submission UUID. The server obtains the Steam ID from the RPC sender; the Agent obtains the DayZ server ID from its configuration. Each complete JSON request is published with a `.ready` marker, and responses are published by renaming a temporary file. The Agent uses a separate creation worker so Discord latency does not block account-link checks. The form keeps the original submission ID and text for retries during the current game session. An unconfirmed request remains locked; Retry checks the same operation. A validation rejection before dispatch explicitly allows editing; local validation runs before a request is sent. UUID validation accepts the uppercase IDs generated by Stable DayZ. A confirmed success allows Open conversation. Closing and reopening the form in the same session restores the submission. Backend, Agent and gameplay scripts must all run this updated version; restart the backend/Agent and rebuild the mod for a packed launch, or restart the diagnostic game instances when using file patching.

The ticket form has no subject field. The type selector shows only ticket-type names; panel IDs remain internal for routing. Include player info is unchecked by default and follows the saved submission when retrying. When selected, the game server captures position, health, blood, shock, water, food, bleeding wounds and unconsciousness from the authenticated sender's character. The client only supplies the choice, never the values. The Agent forwards a snapshot only when the explicit opt-in flag is true, including when native JSON writes an empty object for a null reference. The Discord opening context includes the verified Steam ID and profile link. Position and condition remain opt-in; automatic player-template enrichment stays disabled. No IP address is added.

Ticket-form verification: 186 Agent tests and 114 focused backend tests pass. Isolated Stable server/client probes confirm valid native uppercase UUIDs, no required subject, opt-in default off, editable validation rejections, locked unknown delivery status, correct capture of synthetic character condition and the native form layout. The Discord opening-message renderer is checked with synthetic data; no live ticket or message is sent by these tests. Backend, Agent and gameplay scripts must be updated together before a live check.
Ticket-list verification: 183 Agent tests passed, including list mapping, account-link preservation on list errors and cancellation. An isolated Stable diagnostic server/client loaded the JSON model, and the client created the real layouts with synthetic ticket data. Row selection, summary rendering, Back, a ticket disappearing after refresh, linking, list errors and an empty list passed. No live Discord ticket was created or queried by this probe. The updated backend, Agent and gameplay scripts still need to be run together for a live check.


## Discord message rendering

The Agent parses Discord text with Markdig and sends structured text runs through the existing conversation bridge. The game draws plain TextWidgets and ImageWidgets, rather than interpreting Discord messages as engine markup. Literal angle brackets and ampersands therefore remain text. User mentions use Discord metadata or the verified ticket owner's name; unresolved references have readable labels instead of raw IDs.

Supported presentation includes nested bold/italic/underline/strikethrough, headings, small text, lists, quotes, inline/fenced code, HTTP(S) links, UTC/relative timestamps and click-to-reveal spoilers. Code remains literal. Excessive Markdown nesting falls back to readable text for that message. External URLs are opened only when the player clicks a link.

Unicode emoji use 4,009 bundled Twemoji 17.0.3 images in six PAA atlases (about 2.6 MB total). Skin tones, flags, keycaps and family sequences are matched as whole graphemes. Unknown emoji retain a text fallback; custom Discord server emoji are shown as :name: when no bundled image exists. Custom animations, video previews and code syntax highlighting are not reproduced. Attribution and the graphics license are included under gui/emoji.

The native message view measures words and groups adjacent text with the same style into a single widget per visual line. Wrapping follows the available width. Unchanged histories do not rebuild the message widgets. The image sets load once when opening the conversation; no additional DayZ mod or runtime image download is required.

Conversation headers show the player's local system time, with a date for older messages. UTC remains the storage and transport format. DayZ exposes the current local/UTC offset, rather than historical time-zone rules; older messages use that current offset too.

The in-game emoji picker offers all 3,953 Unicode 17 keyboard entries (3,944 emoji plus nine skin/hair components), including 270 flags. Search covers English names and shortcodes across ten tabs. The complete grid is created once with batched layout updates, then filtered in place by category and search. There are no pages; every result is accessible by scrolling. Hover shows the name and shortcode. Choosing an image inserts its readable shortcode into the native editor. Escape closes the picker before leaving the conversation. The Agent expands the same generated shortcodes for Discord using the Markdown parser, preserving escaped codes and code blocks; the backend forwards that prepared reply without another emoji replacement.

The composer uses DayZ's fixed MetronBook16 editor font, with emoji and send actions inside the rounded field. Its vertical scroll range is clamped to the text and caret; the input scrollbar is transparent, while the message scrollbar uses the support theme. DayZ's native multiline editor does not soft-wrap paragraphs: Enter starts another line, and the editor preserves the draft without inserting automatic line breaks. UI updates only change enable states and labels when necessary. A send requested during a history refresh waits for that read to finish, preserving the server's single-request-per-player limit and the original reply ID. In-game bot replies display the ticket owner's name even when Discord omits the nonce from history; ordinary user messages cannot claim that attribution through an embed name.

Rebuild the mod with F8 and restart both diagnostic game instances after changing the message model. Run the matching backend and Agent builds together. The addon include list must retain PAA, imageset, styles and text files so emoji assets and attribution are packed. To update the pinned emoji assets, use workbench/Build-EmojiAssets.ps1 and copy its generated EmojiCatalog.tsv into the Agent's DayZ/Formatting directory.
## Ticket notifications

Online ticket replies use the native DayZ notification system for ten seconds.
The default shortcut is J, configurable as **Open notified support ticket** under
**Controls > ServerSentry**. It opens the exact ticket while the notification is
active. The timer starts when a queued notification becomes visible. Replies to
an already-open ticket refresh the conversation without a popup. Own messages,
bot/webhook echoes and offline/stale recipients are ignored. Notifications arrive
through the existing Discord gateway and authenticated Agent connection, then a
small `$profile:ServerSentry/tickets/notifications` inbox; conversations are not
polled in the background. Run matching backend, Agent and Mod builds together.

Receiving a notification preloads its conversation through the same authorized
read path used by the menu. J reuses that request/history. The underlying Tickets
screen loads its list only when it is the visible menu. If another read or send
is active, the latest requested ticket starts as soon as that operation finishes;
a notification also schedules a fresh read after an older read of the same ticket.

## Ticket creation

Ticket creation requests only account-link state and allowed ticket types; it
does not reload the player's ticket list. The Agent keeps successful linked
results in memory for at most 60 seconds, scoped by configured server ID and
Steam ID, with at most 256 entries. Responses carry the remaining lifetime so
the client can reuse types without extending that lifetime. Unlinked/error
results are not cached. **Check again** in the form and **Refresh** in the list
bypass the options cache. Lists themselves are always queried fresh. Ticket
creation still uses the existing CMS link/role/type validation. Deploy the
matching Mod and Agent builds together for the lightweight request intent.

## Editor stability

Enable **Workbench → Options → General → V Sync** and leave **FPS Limit Enabled** off. The launcher also passes `-limitFPS=60`. This reduced measured layout-preview CPU usage, but native Qt crashes remain unresolved. Use a full restart after changing editor scripts; Reload Scripts has triggered a native script-leak assertion. The extracted editor-only CF copy skips gameplay module registration under `WORKBENCH`; installed CF and the gameplay PBO remain unchanged.

Cleanup verification: fresh Stable Workbench startup with mod sources and launch/build plugins visible; both support layouts opened in Layout Editor; Mission scripts compiled on server and client; the client joined the local server; pboProject completed with exit code 0. Item delivery and ticket submission were not exercised.

Addon Builder verification: selected ADDON_BUILDER in the saved build settings and built successfully from Script Editor with F8. Repeated with a running local diagnostic server holding the PBO open: F8 stopped the server and completed the build. Extracted the PBO with BankRev; all 180 staged scripts, configs, layouts, inputs and textures were present with matching file hashes under the `ServerSentryDayZMod` prefix.

Launch verification: the local server/client launch plugin ran through Resource Manager, generated server arguments before client arguments, compiled both Mission modules and connected the client to port 2322. The missing-project error was fixed by starting in the installed Workbench directory. F6 was explicitly assigned to Script Editor Plugins > Launch Game, replacing the conflicting Console Execute shortcut.

Conversation navigation uses Back, Refresh and Discord image icons in the header. Discord stays outside the composer. The sidebar centers a single-level, uncompressed 128px logo at its display size, followed by a single-line brand name and a rounded section row. The composer reserves a separate action row below the clipped text viewport. Support screens share cyan, blue and violet accents on dark surfaces. To regenerate the tightly framed 128px logo and navigation icons, run workbench/Build-SupportBrandAssets.ps1; its source logo lives in workbench/assets and is excluded from the game PBO.

The shared rounded skin uses uncompressed alpha masks with 8x8 supersampled edge coverage and symmetric geometry in all four corners. Icons are antialiased into 32px uncompressed textures and displayed without UI scaling. Close uses the same icon path as the other header actions. Regenerate it with workbench/Build-SupportSkin.ps1. The game keeps normal texture filtering enabled for antialiasing; the logo has no DXT compression or lower-resolution mipmaps.
