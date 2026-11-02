# Ticket notifications

Run `init.c` as an isolated offline ChernarusPlus mission with CF and a freshly
built ServerSentry PBO. Use a separate profile and omit `-filePatching` to exercise
the resources inside the PBO. No real tickets or Discord messages are created.

The fixture checks notification ownership, duplicate suppression, the ten-second
shortcut, native queue promotion, expiry, inbox consumption and direct ticket
opening. Opening a ticket must emit exactly one native removal event, remove its
owned popup and preserve unrelated notifications. Stopping must remove both
visible and deferred owned popups. Repeated consumption must be harmless.

It also loads every support skin texture through a real native ImageWidget,
checks its dimensions, checks all six emoji atlases/imagesets and requires the
3,953-entry runtime picker catalog. Excluding `EmojiPicker.tsv` must fail this check even
when the texture files are present. Success ends with
`[TicketNotificationsReview] Finished failures=0`.

For a physical keyboard check, receive a fresh reply while the conversation is
closed, press the configured shortcut within ten seconds and verify that the
exact conversation opens. Its notification should immediately start the normal
DayZ fade-out. Other notifications must remain visible. Repeat with a full native
notification queue and with the ticket already open.

On 2 November 2026, all 60 checks passed with Stable 1.30's normal `DayZ_x64.exe`,
a separate offline profile, a Bohemia-built PBO and no file patching. A separate
comparison with the local RaG output loaded all six support icons and all six
emoji atlases/imagesets, but returned zero picker entries: that PBO excluded
`EmojiPicker.txt`. The complete PBO returned all 3,953 entries. This establishes
the catalog packaging failure; it does not establish the cause of a different
notification appearance with a server's complete third-party mod set.

After renaming the runtime catalog to `EmojiPicker.tsv`, the same 60 checks passed
again in Stable's normal client without file patching. The test PBO contained the
TSV catalog and no legacy `EmojiPicker.txt`; all 3,953 entries loaded successfully.
