# ServerSentry Workbench

Use **Start-Workbench.cmd**. This setup uses Stable DayZ / Workbench 1.30.164200 and the Dabs Workbench tools. Client, server and build settings are managed inside Script Editor; there are no separate BAT launch configurations.

| Shortcut | Workbench command |
| --- | --- |
| F8 | Build Mod using the configured packer (pboProject by default). |
| F6 | Restart the local Stable server, then start a diagnostic client that connects automatically. Selects the server debugger. |
| Shift+F6 | Launch Server and select the server debugger. |
| Ctrl+F6 | Launch Client and select the client debugger. |
| Ctrl+Shift+F10 | Close this project's diagnostic client/server. |
| Ctrl+Shift+F6 | Configure Project: launch paths, mods, profiles, port and arguments. |
| Ctrl+F8 | Configure Build Game: packer and build arguments. |

Close the diagnostic game instances before F8, then launch again. F6 starts a real multiplayer session using DayZDiag with the mod and file patching; it does not load an offline mission. F6 restarts only this project's diagnostic instances and defaults to server debugging. To debug client-side UI/scripts instead, choose **Debug → Debug Client**; choose **Debug → Debug Server** to switch back. Only one target is selected at a time. Debug Server uses port 1002; Debug Client uses 1001. F9 toggles breakpoints, F5 continues, F11 steps into and F10 steps over. Workbench may request authorization for its local scripts' file/process operations; handle those dialogs manually.

For layouts, open **@ServerSentrySources → ServerSentryDayZMod → gui** in the main Resource Browser. Both support menus use stock DayZ widgets/fonts with shared button states and scrollable item rows. Dabs/COT are not UI runtime dependencies. The mod's existing CF dependency remains.

F6 must be reserved for the launch plugin. In **Workbench → Options → Shortcuts**, clear the native **Script Editor → Console Execute** binding if it is also F6. This conflict was cleared in the local Workbench preferences; the console remains available through its Run button.

## Files and output

- `workbench/project.cfg`: mods and PBO prefix used by the Dabs tools.
- `workbench/launch.bin` and `build.bin`: saved Workbench dialog settings.
- `workbench/serverDZ.cfg`: local server configuration; copied to `server.cfg`. Keep `allowFilePatching = 1`.
- `.workbench/@ServerSentry/addons`: development PBO; `.workbench/mods/@ServerSentryDayZMod` points to this mod.
- `.workbench/@ServerSentrySources`: unpacked editor wrapper; its scripts/gui junctions edit the repository directly and allow repacking while Workbench is open.
- `.workbench/profiles/ServerSentryDayZMod/client` and `/server`: game profiles and logs with the default launch settings.
- `.workbench/mpmissions/ServerSentryDayZMod.ChernarusPlus`: isolated local mission/persistence.
- `.workbench/vendor/DabsFramework`: unchanged upstream checkout/license, pinned to `245190266b4e72bb0519fbfdd68bbed71c1edc1f`.
- `.workbench/dabs-tools` and `vanilla-stable/scripts`: generated compatibility copies for the editor. They are excluded from the gameplay PBO.

`Start-Dev.ps1` prepares/opens Workbench, stages game files for the build plugin, or stops this project's diagnostic processes. `Start-NativeGame.ps1` launches the plugin-generated arguments from the game's installation directory. The launcher uses the installed native Workbench project, an unpacked mod wrapper and one isolated editor-script copy; it does not generate a second project or editor mod. Generated Enforce scripts use UTF-8 without a BOM for Windows PowerShell 5 compatibility.

Workbench starts with its installed `Bin/Workbench` directory as the working directory so it can find the native `dayz.gproj`. The explicit `-repository` argument keeps mod-specific launch/build settings in this repository's `workbench` folder. Startup plugins run through Resource Manager with `-plugin=ClassName`; Initialize ServerSentry opens Script Editor.

The ESC Support button and hotkey request the player's support state from the server. An existing case opens reimbursements; otherwise the new-ticket form opens. Responses use request tokens, a timeout and parent-menu checks. Empty cases cannot be claimed. Ticket submission remains disabled until the ServerSentry backend is connected. Item delivery still uses the existing server implementation; the UI does not claim a delivery acknowledgement that the server does not send.

## Editor stability

Enable **Workbench → Options → General → V Sync** and leave **FPS Limit Enabled** off. The launcher also passes `-limitFPS=60`. This reduced measured layout-preview CPU usage, but native Qt crashes remain unresolved. Use a full restart after changing editor scripts; Reload Scripts has triggered a native script-leak assertion. The extracted editor-only CF copy skips gameplay module registration under `WORKBENCH`; installed CF and the gameplay PBO remain unchanged.

Cleanup verification: fresh Stable Workbench startup with mod sources and launch/build plugins visible; both support layouts opened in Layout Editor; Mission scripts compiled on server and client; the client joined the local server; pboProject completed with exit code 0. Item delivery and ticket submission were not exercised.

Launch verification: the local server/client launch plugin ran through Resource Manager, generated server arguments before client arguments, compiled both Mission modules and connected the client to port 2322. The missing-project error was fixed by starting in the installed Workbench directory. F6 was explicitly assigned to Script Editor Plugins > Launch Game, replacing the conflicting Console Execute shortcut.
