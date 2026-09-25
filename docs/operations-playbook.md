# Reforger operations playbook

This is the working record for making local mod experiments repeatable. It records what we observed on this Windows machine separately from commands documented by Bohemia. Update an entry after a real run; do not treat a Workshop description as a gameplay test.

## Current machine

| Component | Local state on 2026-09-24 |
| --- | --- |
| Game | Stable 1.8.0.13, `D:\SteamLibrary\steamapps\common\Arma Reforger\ArmaReforgerSteam.exe` |
| Workbench | `D:\SteamLibrary\steamapps\common\Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe` |
| Client Workshop cache | `C:\Users\marco\Documents\My Games\ArmaReforger\addons` |
| Client logs | `C:\Users\marco\Documents\My Games\ArmaReforger\logs` |
| Dedicated server | Installed separately at `D:\SteamLibrary\reforger-automation\server\ArmaReforgerServer.exe`; version 1.8.0.13 matches the client |
| SteamCMD | Installed under the task-specific `D:\SteamLibrary\reforger-automation` area; exact executable path is recorded by the setup command |

Use `npm run cli -- doctor --mod 6A3A112604EF7286` to check configured roots and local mod presence. This machine's ignored config includes `.cache/workshop-fetch/profile/addons` as a mod root, so the doctor reports Raven as installed even though the normal client Workshop cache does not contain it. The helper also checks this machine's separate server install; use `REFORGER_SERVER_EXE` to override it if the server moves. The ignored `reforger-agent.config.json` is specific to this machine; edit the example config before using it elsewhere.

For a new Workshop ID, use the one-command fetcher. It generates a private server config, uses a reusable isolated cache, verifies the requested addon GUID and nonempty `data.pak`, and stops the server after a bounded run. It dry-runs unless `--execute` is given:

```powershell
npm run mod:fetch -- --mod '6A3A112604EF7286=Raven AI Commander'
npm run mod:fetch -- --mod '6A3A112604EF7286=Raven AI Commander' --duration-seconds 30 --execute
```

A fresh live fetch of Raven completed through a verified `127.0.0.1:25531` listener and left no server process or listener. The reusable cache is `.cache/workshop-fetch/profile/addons`. For large or slow mods, increase `--duration-seconds`; the helper verifies the pack only after the bounded run. Required dependencies may be downloaded by the server but are not reported as requested IDs by the helper.

## UI observations from a real session

1. Launching the game through Steam returned before the Reforger window appeared. The splash screen then remained visible for roughly 20 seconds before the menu. Check for the actual game window instead of assuming an immediate launch failure.
2. Main menu **Workshop** opens the in-game browser. Enter a mod name in its search field and press **Enter**. During this session, clicking the magnifying-glass icon did not apply the query. The results appeared after Enter and a short delay; confirm the result counter and mod title before selecting a tile.
3. The game's search field accepted individual key events. Clipboard-style text insertion through the desktop controller did not populate it in this session. Recheck the visible text before searching.
4. Selecting the **Raven AI Commander** tile opened its detail page and exposed a download icon in the upper-left content area. The client download manager opened by clicking the on-screen **Downloads** control. Its Active and History tabs were empty in the first profile, where Raven existed only in a separate server cache. In a later fresh isolated profile, clicking the detail-page download icon changed **Downloading 0%** to green **Enabled**. The game's Workshop download placed a `data.pak` in that profile's `addons` folder; its SHA-256 matched the previously server-fetched pack. This was the first verified client Workshop installation of Raven in an isolated profile.
5. In **Workshop → Downloaded**, the search field also needed individual key events and **Enter**. Searching for Raven returned **0 / 272** downloaded entries, consistent with the separate server cache. The on-screen Downloads and Back controls responded to clicks when their displayed Tab and Escape shortcut keys did not respond to desktop-controller key injection in this session.
6. **Workshop → Mod manager** has separate Disabled and Enabled columns. The existing client profile contains many enabled mods; do not use that set as a clean baseline. The Downloaded tab also showed several entries with missing dependencies. Avoid bulk toggles while building a focused test.
7. A featured **Game Master - Kolguyev** tile on the main menu launched the scenario immediately. With the existing enabled set, loading failed: `Scripts/Game/SpearHead/XP/Rewards/SH_SCR_EXPRewards.c(1)` reported a second declaration of `SCR_EXPRewards.DEATH`. Aborting produced a session error and a list of selected mods before returning to the menu. This is a reproducible client-profile conflict, not evidence that vanilla Game Master is broken.
8. The main menu **Scenarios** tile opened the scenario browser instead of launching immediately. It displayed named scenarios, source, player count, search, and a per-row Play control. Use this view when choosing a scenario through the UI; use `game.scenarioId` in the server JSON for repeatable scripted setup.
9. Launching a fresh client profile initially showed a **Press Enter** splash and a first-run tutorial prompt. Desktop-controller input did not advance the splash immediately; after the client finished its startup, clicking **Cancel** on the tutorial and **No** on the optional training prompt reached the normal menu. A featured Game Master Kolguyev launch then loaded successfully and displayed Game Master controls. The original profile's duplicate script error did not recur.
10. A later clean-profile attempt to use in-game **Direct Connect** stayed visually on **Press Enter to continue** even after the client log said it had entered the main menu. Targeted Enter and clicks did not reliably advance the splash; a subsequent input surfaced the Codex window over the game. UI automation stopped there. The log had no multiplayer connection attempt, so this run says nothing about whether Direct Connect itself works. Refresh the target window/focus before more input, and stop rather than keep sending keys when focus is uncertain.
11. Launching a world directly avoids that splash. The isolated client opened `worlds/GameMaster/GM_Arland.ent` twice on separate clean profiles, displayed Game Master controls and the entity browser, and responded to a targeted in-game **Hide** click. Both logs show the requested world and a `GAME` state. The tested path is a local Game Master world, not a multiplayer scenario join.

Bohemia's `-noSplash` parameter skips loading splash screens, not the interactive **Press Enter** gate. `Entered main menu` in a log means the world initialized; it does not prove the gate was dismissed. There is no documented startup flag that bypasses this menu interaction.

For client gameplay tests, use an isolated enabled-mod set. A joining client should acquire server-required mods according to Bohemia's documentation, but joining this local server has not yet worked in our test. Do not infer a successful client install from a server cache download. Test one AI pilot mod at a time. Bohemia recommends isolating mod conflicts this way.

If a UI action stalls, record the exact screen and result here. Prefer a verified keyboard or command-line path over repeating uncertain clicks.

## Desktop-control setup and observation loop

Keep two monitors if convenient. The Windows controller captures the selected Reforger window, not the combined desktop, so the second monitor does not enlarge each game screenshot. Its click coordinates are relative to that window. In a live Game Master capture, the target image was 1282 × 752 at screen origin `(319, 329)`. The client launcher requests a stable 1280 × 720 window; keep its size and position fixed during a test, and put Codex and logs on the other screen. Avoid clicking or moving the game window while automated input is in progress, because focus still matters.

When the user reported that Reforger looked minimized, the window controller still found one Reforger window. Its first capture showed Steam content over the same region; activating the returned Reforger window brought the live Game Master world into view without restoring or relaunching it. That run supports **covered by another window** rather than a confirmed minimized state. Check the target window and activate it before changing launch flags or restarting. Foreground activation did not make the separate **Press Enter** splash respond, so keep using direct `--world` startup for the local gameplay loop.

For each player-style test, capture the target window, choose one visible action, send one input, then capture a fresh state before choosing the next input. Refresh the selected window handle when a launcher changes to another editor, or when a modal takes focus. If a click targets the wrong window or the UI does not change, stop and reacquire the game window rather than repeating coordinates. Use the current client log to confirm the world reached `GAME` and to correlate any editor/script events; a screenshot confirms what the player saw. Record both observations, the exact control used, and whether the intended in-game state changed. The screenshot and log prove different things, so neither alone should be called a completed gameplay test.

Summarize one run's log with `npm run logs:summary -- --log <console.log>`; add `--json` for structured output. It reports the latest mission, `GAME`, loaded addons, Raven startup and `SUPPLY_RUN` events, and join failures with line numbers. A live summary of the player-hosted Raven Conflict log found the correct mission and Raven ready with **zero** supply-run events at that point. The helper deliberately does not infer a completed delivery from a claim or log-reported transfer; verify vehicle behavior and before/after source and destination resource counts in the UI. It reads the selected log as a point-in-time file, so rerun it after further play.

## Workbench UI observations

1. Launch **Arma Reforger Tools** from Steam and allow for startup. The Enfusion Workbench Launcher appeared after roughly 14 seconds in this run; it was absent from the first window listing and present on the next. Check again before treating the launch as a failure.
2. In the launcher, **Add Project** is a split button. Its dropdown appeared outside the launcher's captured view and accessibility tree. From the first menu item, one **Down** key highlighted **Add Existing Project**; **Return** opened the `.gproj` file picker. The picker initially showed the previously used Workshop mod folder, so navigate to the intended project explicitly.
3. The existing project is `C:\Users\marco\Documents\My Games\ArmaReforgerWorkbench\addons\New Enfusion Project\addon.gproj`. After adding it, select the project in the launcher and choose **Open**. This replaced the launcher with **Enfusion Workbench - New Enfusion Project**; automation must refresh the window list and acquire the new window handle.
4. Resource Manager reached **Ready** without a dependency prompt. Its browser listed `ArmaReforger`, `core`, `logs`, `NewEnfusionProject`, and `profile` roots. The home view provided **World Editor** and **Script Editor** launch tiles, and the **Editors** menu offered the same paths. This UI check made no project edits and closed Workbench afterward.
5. On the next launch, the project was already listed and selected in the launcher. **Open** brought up Resource Manager, but it restored prior prefab viewer tabs instead of the welcome tiles. Use the **Editors** menu when the home tiles are absent. Its popup appeared in screenshots but did not immediately appear in the accessibility tree. From the newly opened menu, one **Down** selected the first item; **Down × 4**, then **Return**, opened **Script Editor** as a separate top-level window. The Script Editor showed a Projects tree and empty Errors pane; no document was edited.
6. Re-activate Resource Manager and open **Editors** again. One **Down**, then **Return**, opened **World Editor - Unnamed empty world** as another top-level window. The hierarchy showed `unnamed/default`; **Load World** and **Save World** were available. No world was loaded or saved. The Resource Manager accessibility tree can still describe the old window while an editor visually overlays it; use the current window list and bind the new editor window. `Alt+F4` closed World Editor, Script Editor, then Resource Manager in the observed run.

For repeatable validation and packaging, use the command-line helper below. To open an editor without navigating the launcher, use the dry-run-by-default `workbench:open` helper:

```powershell
npm run workbench:open -- --editor script
npm run workbench:open -- --editor script --execute
npm run workbench:open -- --editor world --world worlds/GameMaster/GM_Arland.ent --execute
```

It resolves the selected project, installed Workbench, and base-game addons, gives each GUI run its own logs, and supports `resource`, `script`, or `world`. The Script Editor command's underlying `-gproj <addon.gproj> -addonsDir <base game addons> -wbModule=ScriptEditor -run` flags were live-tested: they bypassed the launcher and opened Resource Manager plus Script Editor with no prompt. The helper's `--execute` and World Editor `-load` path also opened GM Arland for the Convoy Follower scene setup. Select the returned World Editor window after launch; the launcher may have closed or changed windows.

## Convoy Follower Workbench test scene

The first addon is `addons/ConvoyFollower/addon.gproj`. Its prepared world resource is `Worlds/Tests/ConvoyFollower_Arland.ent`, a subscene of Game Master Arland. In World Editor, all four placed entities were visible on the US base apron: a US spawn point, the custom one-member Convoy Driver group, and two M923A1 transport trucks at approximately `(1160, 1, 3325)`. The nearer truck is intended for the AI and the other for the player. The scene was saved, then passed fresh Workbench validation and packaging; the packaged `data.pak` contains the scene. Pass the project explicitly; `workbench:open` still defaults to the older New Enfusion Project.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland.ent
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland.ent --execute
npm run workbench -- validate --project addons/ConvoyFollower/addon.gproj --execute
```

The first command is a dry run. The World Editor `-load` path worked for both the parent GM Arland world and the saved test subscene. On reopening, the camera returned to its previous forest view; expand the `ConvoyFollower_Arland` layer, select `M923A1_transport1`, and press `F` to focus the prepared apron. An earlier F5 preview displayed the built-in **Script Authorization Required** prompt for vanilla `SCR_ArsenalManagerComponent.c` / `BaseContainerTools.LoadContainer`; later user tests reached gameplay and the driver order. The revised source was packaged successfully as `.cache/local-addons/ConvoyFollower/data.pak`; the pack log is under `.cache/workbench-runs/convoy-multidriver-pack`. To test it, manually load supplies, approach the driver, select **Drive closest empty vehicle and follow me**, wait for it to board, then enter the other truck and lead it along a road. The source intends exiting the lead vehicle to pause the follower in its seat and re-entering to resume; use an explicit **Stand down** order to end the task. The exit-and-resume sequence has not yet had a live retest. Record visible movement and any stuck recovery alongside `[ConvoyFollower]` log lines. The detailed steps and current distance/time limits are in [the addon README](../addons/ConvoyFollower/README.md). Single-vehicle following was visually confirmed in a truck and a Jeep, and two trucks visibly followed in a later test; cargo delivery and the separate Game Master catalog card still need checks.

Live user feedback: selecting the earlier **Drive nearest truck and follow me** action sent the driver into the player's Jeep. The implementation chooses the nearest empty wheeled vehicle within 35 m. The source action label is now **Drive closest empty vehicle and follow me**; the target can still be misassigned when several vehicles are nearby. The user wants to keep this mechanic for the current test. A later design pass should allow a simple explicit vehicle designation and show or confirm the chosen target. The user visually saw a single follower driving behind the lead vehicle in a truck, then saw the Jeep driver physically follow in another run. In Workbench `script.log`, the first order logged `BOARDING` (15:10:29), `BOARDED` (15:10:39), `FOLLOWING` (15:10:44), then a stalled `RETRY` (15:11:14). A later group/order logged `BOARDING` (15:14:52), `BOARDED`/`FOLLOWING` (15:15:02), `ARRIVED`/`STAND_DOWN` (15:15:22), and `DISEMBARKED` (15:15:26). Group spawn entries were at 15:09:25 and 15:13:59. The logs confirm state changes; the user observation establishes physical following in those single-driver runs. Neither establishes cargo delivery.

Concurrent-follower failure in the older tested build: the player exited the lead vehicle to order a second driver. The first driver then automatically stood down and dismounted. Repeating the order process alternated which driver followed instead of keeping both active. The revised source pauses while the lead player is on foot, resumes after that same player enters a separate lead vehicle again, and reserves stand-down for an explicit order. The new pack contains this change, but the exit-and-resume sequence still needs a live test.

The current packaged build (`.cache/workbench-runs/convoy-combat-reboard-pack`) uses one server-owned convoy per player: Unit One follows the player, Unit Two follows Unit One's truck, and later units follow the immediate predecessor. It has Start/Add/Replace/owner-checked Stand down and Follow/Stop on-foot actions, plus a three-driver, four-truck test scene reopened in World Editor. Only the leader's processed `unit.wav` calls are routed privately to the ordering player; numbered exception clips cover Units Two through Ten. The chosen thresholds remain a warning after 3 seconds at or beyond 180 m and lost after 10 seconds beyond 275 m, measured per link. The current source listens for confirmed projectile/explosive hits on an assigned truck or driver to request an under-fire call; near misses do not qualify. `docs/convoy-leader-assets.json` records the 58 processed audio resources, with stronger voice treatment and opening/closing radio noise. The prior chain/audio behavior was live tested. A polish preview logged passenger riding and stopped-position waypoint updates, but under hostile fire the player saw all three drivers exit and the convoy dissolved. The current build raises active driving order priority, retries unexpected dismounts, and attaches damage listeners after initialization; Workbench validation and packaging passed, and the reopened scene logged all three driver listeners attached. The combat, hit call, multiwalker, visual spacing, gear, and terminal-dismount behavior need a fresh live pass; see the addon README.

The player tested the earlier chain build immediately after handoff. Three drivers boarded, then three trucks visibly followed in the intended chain. The leader's processed voice calls were audible and the actions behaved as expected. Live logs recorded Unit Two and Three range/stall exceptions and Unit One's range warning/lost transition; logs alone do not prove exact vehicle positions. The player asked for a closer driving gap. They also ordered two idle drivers to follow on foot and saw only one walking, despite two `FOOT_FOLLOWING` starts in the log. The refreshed build replaces near-completing MOVE orders with entity FOLLOW orders, isolates each driver's waypoint, adds passenger transport, hit reports, US crew/M9 gear, a closer 12 m driving gap plus 10 m stopped settle, and dismounts after terminal stuck retries or explicit vehicle destruction. The new World Editor process loaded the same test world after a successful pack; these changes still need visual/audio checks.

The September Everon staging pass added a useful World Editor check before any F5 preview. Double-click a placed entity in the hierarchy to frame it, then scroll outward over the viewport to see nearby vehicles and obstacles; clicking bare terrain exposes approximate world coordinates in the status bar. This caught an initial truck line intersecting brush, a tree, and rocks even though headless validation and packing passed. The revised Regina sports-ground line and Levie storage are visually clear in World Editor. The final combined pack is `.cache/local-addons/ConvoyFollower/data.pak`, with logs at `.cache/workbench-runs/convoy-everon-soviet-seat-final-pack-v2`; five configuration checks passed. The new LAV spawn prefab attempts its named vehicle's driver seat and falls back on foot, but needs live confirmation. The Everon mission header requests 22:00; bare F5 may inherit daytime. For this explicitly authorized local test environment, `workbench:open --authorize-local-test-scripts` passes the documented `-scriptAuthorizeAll` switch. The flag is opt-in and applies only to that Workbench launch; normal launches retain the prompt.

The September 25 radio pass removed the **Turn convoy voice off/on** action from both driver prefabs. Authors now set `m_bVoiceEnabled` in `Configs/CF_ConvoySettings.conf` and rebuild the addon. Routine calls keep their random chance and gain server-side spacing and per-phrase repeat limits; range-warning and rejoined repeats are also debounced. Stuck, lost, and under-fire reports remain unconditional. The configured convoy limit is now hard-capped at five. A unique Workbench validate and probe pack completed without addon errors in `.cache/workbench-runs/convoy-voice-settings-pack`; these audio changes still need an in-game listening test.

### September 25 autonomous preview finding

`npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland.ent --authorize-local-test-scripts --execute` opened the World Editor. F5 then reached the embedded Game Master preview without the script authorization dialog. The first startup raised an Enfusion `Resources are leaking!` assertion while game scripts reloaded; its log is under `.cache/workbench-gui-runs/2026-09-25T05-52-07-855Z-22972`. After dismissing that assertion, a second F5 reached playable Game Master. Treat a repeated assertion as a separate engine failure, not as successful mod testing.

With a fresh Game Master scene, the large `+` at lower left opens scenario properties. Enable the US flag under **Playable factions**, then **Save and close**. The upper-left **Respawn menu** opens deployment; choose **US Army**, join **Atlas Red 4**, continue, and deploy at **US Point**. These mouse actions worked through window-targeted desktop control and spawned a player beside the staged M923. Momentary key inputs through the Workbench window have not proved reliable sustained movement or driving, so the autonomous road test needs a scripted lead vehicle; a screenshot of the spawned player alone is not pathing evidence.

In the embedded Game Master preview, a second F5 did **not** stop the active preview. Use **Game → Exit game (Esc)** from Workbench instead. That transition raised a `Resources are leaking!` Enfusion diagnostic once in the live run; **Ignore** returned to World Editor. Select the intended auto world again before the next F5 and use a fresh log. See [the autonomous smoke procedure](convoy-automated-smoke.md) for the separate one-, two-, and three-truck scenes. The first live one-truck auto run bypassed the playable-faction menu, possessed and seated a test character, and issued the real convoy order, but its direct simulation throttle did not move the lead truck. Its `AUTO_RESULT` was `FAIL lead moved less than 20 m`; do not label that run a driving pass. Follow-up diagnostics now log engine, gear, brakes, and input state.

## What the command line can do

Bohemia documents these flags in [Startup Parameters](https://community.bistudio.com/wiki/Arma_Reforger%3AStartup_Parameters). Quote paths with spaces. Run from a PowerShell terminal, substituting actual executable and scenario paths.

```powershell
$gameExe = 'D:\SteamLibrary\steamapps\common\Arma Reforger\ArmaReforgerSteam.exe'
$workbenchExe = 'D:\SteamLibrary\steamapps\common\Arma Reforger Tools\Workbench\ArmaReforgerWorkbenchSteamDiag.exe'

# The game resolves its built-in ./addons relative to the process working directory.
Push-Location (Split-Path $gameExe -Parent)

# Load addons that are already installed and start a particular world.
& $gameExe -addons 'GUID1,GUID2' -world 'worlds/example.ent' -logsDir 'C:\ReforgerTestLogs'

# List available scenario .conf paths in the generated log, then use one in a server config.
& $gameExe -listScenarios -logsDir 'C:\ReforgerTestLogs'
Pop-Location

# Validate scripts in a separate Workbench process. A failed validation should return nonzero.
& $workbenchExe -gproj 'C:\Path\To\addon.gproj' -addonsDir 'D:\SteamLibrary\steamapps\common\Arma Reforger\addons' -wbModule=ScriptEditor -validate PC -wbSilent -logsDir 'C:\ReforgerTestLogs'

# Pack an addon after validation succeeds.
& $workbenchExe -gproj 'C:\Path\To\addon.gproj' -addonsDir 'D:\SteamLibrary\steamapps\common\Arma Reforger\addons' -wbModule=ResourceManager -packAddon -packAddonDir 'C:\Path\To\packed'
```

Workbench can also run a plugin with `-wbModule=ResourceManager -plugin=ClassNamePlugin` and the game can invoke the [Autotest Framework](https://community.bistudio.com/wiki/Arma_Reforger%3AAutotest_Framework) with `-autotest`. Use a separate process for `-validate`; [Bohemia's tracker](https://feedback.bistudio.com/T181951) reports that combining it with other actions could overwrite the exit status. Treat an autotest log as the result until its process exit behavior is verified locally. A live run here initially stalled because the base game dependency was missing; adding the game `addons` directory allowed the existing empty addon to validate successfully. The helper requests `PC` by default, but this installed Workbench build still checked WORKBENCH, PC, XBOX, PS4, and PS5 in a live run, so `PC` did not reduce validation time here.

The client `-addons` flag loads existing local addons; `-addonDownloadDir` only changes the download directory. Bohemia documents Workshop acquisition through the UI or by joining a server that requires mods. There is no documented client flag to download an arbitrary Workshop ID directly. See [Workshop](https://community.bistudio.com/wiki/Arma_Reforger%3AWorkshop).

A live `npm run workbench -- pack --project 'C:\Users\marco\Documents\My Games\ArmaReforgerWorkbench\addons\New Enfusion Project\addon.gproj' --output '.cache\packed-addon' --execute` run validated the existing empty project, produced `addon.gproj`, `resourceDatabase.rdb`, and `data.pak` in the ignored output directory, and logged **Packaging project successful**. The helper now checks for a fresh success line and a fresh, nonempty `data.pak` because Workbench may exit with code zero after a failed pack. This proves the packaging path on this installation; the empty test addon does not exercise custom scripts or assets. The command does not publish to Workshop.

## Isolated client profile

The game also supports `-profile`. It treats the supplied directory as a root: settings go under its `profile` subdirectory and Workshop addons under its `addons` subdirectory. An initial launch from the REFORGER workspace failed with `Game addon '58D0FB3206B6F859' not found` because the game's relative `./addons` lookup used the wrong working directory. Starting from the game executable directory loaded the base game and reached the menu. The `client:run` helper uses that working directory and a windowed test layout by default.

```powershell
npm run client:run -- --profile .cache/client/profiles/logistics
npm run client:run -- --profile .cache/client/profiles/logistics --duration-seconds 600 --execute
```

The first command is a dry run. Reuse the same isolated profile to avoid repeated downloads and preserve its test settings. The helper can load installed addons with `--addon <ID>` and `--addons-dir <directory>`; it checks presence before launch because `-addons` does not download missing mods. For a server on Reforger's default port, `--connect-local` uses the documented `-client 127.0.0.1` flag; the tested automatic join currently times out during authentication. Client CLI syntax for another port and the in-game Direct Connect flow remain unverified here. A live 20-second `client:run` test started and stopped the client cleanly; a separate manual launch with the same profile loaded Game Master Kolguyev.

For a future player handoff, add `--keep-open` to `client:run` instead of a duration. It retains log monitoring and keeps the window running until the player exits or the operator interrupts the launcher with Ctrl+C. This option and its timer exclusion have focused tests; a dry run resolved the registered Raven client profile on this machine, but a live keep-open launch has not yet been run. The Raven trial used a one-hour limit and has ended.

For the fastest verified local gameplay entry, launch Game Master Arland directly with `--world`; Bohemia documents this flag for a `.ent` world resource. It bypassed the menu and reached Game Master on two clean profiles:

```powershell
npm run client:run -- --profile .cache/client/profiles/vanilla-gm --world worlds/GameMaster/GM_Arland.ent --duration-seconds 600 --expect-game --execute
```

The two evidence logs are under `.cache/client/runs/2026-09-24T18-06-53-518Z-24384/logs/console.log` and `.cache/client/runs/2026-09-24T18-07-40-075Z-5404/logs/console.log`. The new optional `--expect-game` check requires a logged transition to `GAME` before the bounded run counts as successful; those prior live runs had the transition, and the check itself has focused tests but has not yet been live-run. It does not prove that a player action worked. `--world` starts a world; it is not a selector for scenario `.conf` IDs. Use the server JSON for a specific multiplayer scenario.

The fastest tested way to load a mod already fetched by the dedicated server is to point the isolated client at that server cache. With `--world`, this loaded Raven's packed addon directly into Game Master Arland without another download:

```powershell
npm run client:run -- --profile .cache/client/profiles/logistics --world worlds/GameMaster/GM_Arland.ent --addon 6A3A112604EF7286 --addons-dir '.cache/workshop-fetch/profile/addons' --duration-seconds 600 --expect-game --execute
```

A bounded run of this combination using the earlier server cache at `D:\SteamLibrary\reforger-automation\smoke-raven\profile\addons` logged Raven in `Loaded addons`, loaded the requested world, reached `GAME`, and showed usable Game Master controls; the tutorial **Hide** button responded to a click. The new fetcher's cache has the same addon layout and was separately verified. Evidence: `.cache/client/runs/2026-09-24T18-09-19-500Z-29480/logs/console.log`. This does not test Raven's supply commands or delivery behavior.

In a later Raven-loaded Game Master session, selecting the bottom **Rifle Squad** card at window-relative `(426, 683)` and clicking open terrain near Arleville at `(697, 508)` placed a US Army AI squad. The selected entity panel showed its AI state as **Idle**, and the client log recorded a `Group_US_RifleSquad.et` spawn at the selected location. This is a verified player-style interaction with matching visual and log evidence. Synthetic `Space` and clicking its **Create player...** hint did not enter a player view, while the mouse card/terrain actions worked. No Raven logistics bootstrap or Raven-labeled control appeared in that Game Master session; Raven's Workshop description targets Conflict, so this does not establish its Conflict behavior. Evidence: `.cache/client/runs/2026-09-24T18-16-39-229Z-28924/logs/console.log`.

An automatic join to a hidden loopback server on the default port was attempted with `--connect-local`. The client targeted `127.0.0.1:2001`, but the handshake timed out; the server logged an authentication failure and refused the connection. A second, vanilla control with matching 1.8.0.13 client/server builds, no mods, and a verified loopback-only listener reproduced the timeout after the server received the client connection. A third no-mods control changed only `publicAddress` to Bohemia's documented `local` value. The server registered its LAN address but kept the game socket bound to `127.0.0.1`; client authentication still timed out. Raven and the loopback advertised address are therefore not required for this failure. The precise cause remains unverified. No joined session was verified. Direct world launch with a local addon cache is the tested gameplay path while this is investigated.

Vanilla control evidence is at `D:\SteamLibrary\reforger-automation\vanilla-control-20260924\server-logs\console.log` and the sibling `client-logs\console.log`. The `publicAddress: local` control is under `D:\SteamLibrary\reforger-automation\vanilla-public-local-20260924` with corresponding server and client logs. All processes were stopped afterward; the default server port had no remaining listener. An in-game Direct Connect attempt could not reach the menu because of the splash/focus issue described above, so only automatic join has a connection result.

After a user manually cleared the **Press Enter** gate in a Raven-loaded isolated profile, mouse navigation reached **Multiplayer → Host → Host new server** and selected **Conflict - Arland**. The Host form offered **Visibility**, **Host LAN**, **Public IP address**, **Bind IP address**, and ports. Visibility was set to **No**, but Bind IP defaulted to `0.0.0.0`; no host was started at that default. The Bind IP field ignored clipboard-style `type_text`, while individual digit key events changed it. A regular `period` key event blurred the field and left an invalid partial address; `KP_Decimal` inserted a dot. Verify the complete visible `127.0.0.1` address before starting a private host. The Host **Mods** tab was empty even though Raven was loaded by `-addonsDir/-addons` from an external cache. The isolated client's own `addons` directory did not contain Raven, so external startup loading alone did not make it selectable in Host Mods in this run.

A second, clean profile copied the complete Raven package into its own `addons` folder before launch, without forcing `--addon` or `--addons-dir`. The client log listed Raven under **Available addons**, but **Loaded addons** still contained only the base game, and both Host Mods columns remained empty. A filesystem copy alone therefore did not register Raven as a selectable Host mod in this test. The entire copied package matched the fetched source pack, including `meta` and manifests.

A third fresh isolated profile, `D:\SteamLibrary\reforger-automation\client-workshop-host-ui-20260924`, downloaded Raven through the **in-game Workshop**. The search required individual `raven` key events and **Return**. The result was **Raven AI Commander** by Miss Violet, version 1.0.6. After its download icon was clicked, the Workshop showed green **Enabled**, and **Multiplayer → Host → Host new server → Mods** listed Raven in the **Disabled** column. The Host was not started during this check. The client log recorded two preview-image move errors, but the package and Host Mods entry were present; do not treat those thumbnail errors alone as a failed addon download. This confirms that the Workshop flow registers host-selectable mods where a filesystem copy did not.

Read-only comparison after the successful download found a per-mod Workshop state record under that profile's `profile\.save\...\workshop\I6A3A112604EF7286.json`, which the copy-only profile lacked. The game also added richer online metadata and preview images. The core `addon.gproj`, `data.pak`, and `resourceDatabase.rdb` hashes matched the earlier server-fetched package. Reuse the Workshop-registered profile for later host tests to avoid repeated downloads; copying only `addons` has not been a working substitute. Bohemia documents client acquisition through Workshop or joining a modded server, but no CLI command to register a server-fetched pack in Host Mods. Manually creating or copying the game's internal Workshop state is not a verified shortcut.

In the same registered profile, the player-host form was set to **Conflict - Arland**, Raven as the sole **Enabled** mod, **Visibility No**, **Host LAN Yes**, **Bind IP 127.0.0.1**, and ports `2001`. The complete Bind IP was visually checked after leaving the field. Clicking **Host** reached **Deployment Setup** and the current client log recorded the Conflict Arland mission, `GAME`, Raven Conflict detection, `AUTO RUNTIME STARTED`, `AUTO BOOTSTRAP COMPLETE`, and `FACTION LOGISTICS READY` for two contexts. The live game process had exactly one UDP listener, `127.0.0.1:2001`. This is the first verified local player-hosted Conflict startup with Raven; it does not prove a supply delivery. Leave the session running when handing it to the player. The previous automatic dedicated-server join timeout does not apply to this successfully hosted path.

In first person at the US Main Operating Base, mouse clicks affected the camera, but targeted desktop-controller key injections for `m`, `Shift+M`, `h`, `Tab`, and `Escape` did not visibly change the view, even after activating and clicking the game window. The HUD showed only a **TAB Quick slots** hint, with no mouse-accessible map or Raven control. After the user pressed physical **M**, the current game log recorded `Creating menu 'MapMenu'`. This supports a raw-input/focus difference between injected and physical keyboard input in the world view; the map itself could not be inspected because the physical Escape stopped the Computer Use controller for that turn. Stop app input when that stop response appears; resume only after a fresh user turn and a new window observation.

The user's own Arland play produced the first Raven convoy claim in this run. At 14:13:13 and 14:13:21 local, the Game Master editor log recorded two `M923A1_transport.et` trucks placed near the US main base. At 14:13:46, Raven logged **Move waypoint READY**, **Supply convoy 1 started**, and `SUPPLY_RUN CLAIMED: task=1`; at 14:13:49 it logged convoy state `2`. Read-only inspection of the installed scripts maps state `2` to the crew switching to its outbound waypoint; it does not establish actual movement. Raven uses its own vehicle bound at startup rather than discovering the player's Game Master trucks, so the timing of those placements does not establish a trigger. No loading, arrival, or resource transfer appeared before the game shut down at 14:15:50. The session ended by game exit, before the original one-hour client runner limit. Preserve its log for the Raven comparison, and do not label a claim or started convoy as a completed delivery.

## One-command private Conflict host

Use `conflict:host` when a player needs time to test a mod in Conflict Arland. It builds the verified Conflict Arland scenario config, keeps visibility off and both addresses on loopback, reuses the isolated `.cache/workshop-fetch/profile` server cache, then calls the same guarded `server:run` launcher. The command defaults to a one-hour bound after the listener opens and stops on Ctrl+C. An explicit `--keep-open` omits the timer for a player handoff and still stops on Ctrl+C. It dry-runs unless `--execute` is supplied:

```powershell
npm run conflict:host -- --mod '6A3A112604EF7286=Raven AI Commander'
npm run conflict:host -- --mod '6A3A112604EF7286=Raven AI Commander' --execute
npm run conflict:host -- --mod '6A3A112604EF7286=Raven AI Commander' --keep-open --execute
```

Add another `--mod ID=Name` for each required Workshop addon. `--scenario` accepts a complete scenario resource ID from `-listScenarios` when the addon targets another Conflict map; `--port` and `--duration-seconds` override the other defaults. The guarded runner prints the exact per-run log directory and verifies that the server process opens only loopback UDP endpoints. The default port is 2001. For a player test, attempt **Multiplayer → Direct Connect** to `127.0.0.1:2001`, or to the chosen port. That UI join path is still unverified; prior automatic local joins timed out during authentication. Confirm the player actually enters Conflict before interpreting any mod behavior. The server cache can provide addons for the dedicated server, while a client testing through the Host Mods UI still needs its own Workshop registration.

The helper has focused config, argument, and dry-run tests. No live server or client join has been run through this new command yet.

## Repeatable server path

A dedicated server is a promising way to request a known set of Workshop mods with JSON instead of navigating Workshop pages each time. Its `game.mods` entries use Workshop IDs, and `game.scenarioId` selects one scenario. The server can download missing mods; a joining client downloads required mods. Obtain scenario IDs from `-listScenarios` rather than guessing them. See [Server Config](https://community.bistudio.com/wiki/Arma_Reforger%3AServer_Config) and [Server Hosting](https://community.bistudio.com/wiki/Arma_Reforger%3AServer_Hosting).

```json
{
  "bindAddress": "127.0.0.1",
  "bindPort": 2001,
  "publicAddress": "127.0.0.1",
  "publicPort": 2001,
  "game": {
    "name": "Local Reforger Test",
    "scenarioId": "REPLACE_WITH_CONF_ID_FROM_LIST_SCENARIOS",
    "maxPlayers": 2,
    "visible": false,
    "gameProperties": { "fastValidation": true },
    "mods": [
      { "modId": "REPLACE_WITH_WORKSHOP_ID", "name": "Readable mod name", "required": true }
    ]
  }
}
```

```powershell
npm run server:config -- --scenario '{C41618FD18E9D714}Missions/23_Campaign_Arland.conf' --mod '6A3A112604EF7286=Raven AI Commander' --port 25531 --out .cache/local-server/raven.json
npm run server:run -- --config .cache/local-server/raven.json --profile .cache/local-server/profiles/raven
npm run server:run -- --config .cache/local-server/raven.json --profile .cache/local-server/profiles/raven --duration-seconds 30 --execute
```

`server:config` validates IDs and refuses to overwrite an existing config without `--force`. `server:run` dry-runs by default, rejects public/ambiguous settings, launches with `-config` when `--execute` is given, and checks the server process's live UDP listeners; it stops a run that binds outside loopback. Use `--profile <directory>` to reuse an isolated server cache across runs. A live bounded run with Raven bound only to `127.0.0.1:25531` and stopped cleanly after its timer.

A short smoke test with the separate `-server` world-launch path **ignored loopback bind flags and listened on all interfaces**; it was stopped immediately. In contrast, a config-based launch bound only to `127.0.0.1` as verified by the server log and live UDP endpoints. The server then downloaded Raven AI Commander and entered Conflict Arland with its logistics system initialized. This verifies acquisition and startup, not a completed supply run. The `-server` world-launch flag ignores `-config`; use one mode at a time. Do not open router ports for this workflow.

Verified scenario IDs from the installed stable game resource database: Conflict Arland `{C41618FD18E9D714}Missions/23_Campaign_Arland.conf`, Game Master Arland `{2BBBE828037C6F4B}Missions/22_GM_Arland.conf`, and HQ Commander Arland `{68D1240A11492545}Missions/23_Campaign_HQC_Arland.conf`. Recheck after game updates.

## Evaluation ladder

Keep the original logistics idea on hold until these comparisons have real gameplay observations.

| Test | Candidate | Pass condition | Record |
| --- | --- | --- | --- |
| Player leads an AI truck | Vanilla squad controls / convoy mods | AI follows a player vehicle to a stop without losing the route | Setup steps, time, stuck behavior |
| Player requests truck delivery | Conflict: HQ Commander; AI Conflict Arland; Raven AI Commander | Player can choose source, destination, amount; supplies arrive after physical travel | Actual controls, source/truck/destination counts, failure recovery |
| Player directs AI helicopter | L63 HeliAI; separately REAPER AiHelicopters | AI starts, takes off, reaches a marked point, lands, and can return | Required entities, commands, landing reliability |
| Player-loaded helicopter cargo | One pilot mod with vanilla supply helicopter | Supplies remain aboard while AI flies and can be unloaded at destination | Vehicle and base resource counts |
| Fully delegated helicopter delivery | Existing mods or a future addon | AI loads, flies, lands, transfers supplies, returns; failure is reported/recovered | Every state transition and recovery outcome |

Primary comparison pages: [AI Conflict Arland](https://reforger.armaplatform.com/workshop/B52C5F6AEDBF423E), [Raven AI Commander](https://reforger.armaplatform.com/workshop/6A3A112604EF7286-RavenAICommander), [L63 HeliAI](https://reforger.armaplatform.com/workshop/D6B177DBEE89E8C4), [REAPER AiHelicopters](https://reforger.armaplatform.com/workshop/69A4D664A6284E06). Their descriptions are claims until tested here. L63 requires BFS F.R.I.E.S Reforged and Physics Rope; keep competing AI pilot mods disabled during individual tests.

For each run, record the game version, enabled mod IDs/versions, scenario ID, exact player actions, elapsed setup time, expected outcome, observed outcome, and log folder. A failure is useful if its reproduction steps are precise.
