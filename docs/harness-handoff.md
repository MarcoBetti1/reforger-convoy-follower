# Reforger harness handoff

## Scope

Use Enfusion Workbench for most addon work. The harness also launches a clean local world or a private Conflict server and collects logs. Current release work requires autonomous one-, two-, and three-truck driving runs with visual and log evidence, followed by a demonstration video. See [the automated smoke-test guide](convoy-automated-smoke.md) for the current test worlds and its limits.

## Working loop

From the repository root in PowerShell:

1. Check the installation and mod cache: `npm run cli -- doctor --mod <WorkshopID>`.
2. Open the addon: `npm run workbench:open -- --editor script --project '<path-to-addon.gproj>' --execute`. Use `--editor resource` or `--editor world` when needed. This helper dry-runs without `--execute`.
3. Validate: `npm run workbench -- validate --project '<path-to-addon.gproj>' --execute`. Package a passing project with `npm run workbench -- pack --project '<path-to-addon.gproj>' --output '<outside-project-directory>' --execute`. The pack helper checks its log and a fresh, nonempty `data.pak`.
4. Launch a clean local Game Master world: `npm run client:run -- --profile .cache/client/profiles/<test> --world worlds/GameMaster/GM_Arland.ent --duration-seconds 600 --expect-game --execute`. Add `--addon <WorkshopID> --addons-dir <directory>` for an installed addon. Use `--keep-open` instead of a duration for a player handoff.
5. For a private Conflict scenario, use `npm run conflict:host -- --mod '<WorkshopID=Name>' --keep-open --execute`. The default is Arland on loopback port 2001; supply a complete `--scenario <resource.conf>` for another map. This is a dedicated server path; the manual client join still needs verification. A player-hosted Conflict session with Raven reached gameplay, but its setup was through the game UI.
6. Summarize a run with `npm run logs:summary -- --log '<run-log>\console.log'`. Keep the exact player actions, visible result, and relevant log lines together. A game or supply-run startup log does not prove a completed delivery.

All launchers dry-run by default. `npm run mod:fetch -- --mod '<WorkshopID=Name>' --execute` obtains a Workshop addon through a temporary loopback server and verifies its package in the reusable cache. A server-fetched package is not automatically registered in the client's Workshop list.

For the local Convoy Follower F5 preview, open the Arland scene or an Arland_Auto variant with `workbench:open --authorize-local-test-scripts --execute`. That opt-in flag adds Workbench's `-scriptAuthorizeAll` switch to this testing process, avoiding the script-refresh prompt. It does not change the default launcher behavior. A previously observed F5 script-reload assertion was a separate Workbench issue; see the [playbook](operations-playbook.md).

## Verified on this machine

- Workbench opened the earlier project and Script Editor. The `workbench:open` helper also launched World Editor directly on GM Arland for a new Convoy Follower subscene. The installed Workbench validated and packed the earlier empty addon and, later, the new addon with its saved test scene.
- A clean client launched Game Master Arland directly and reached playable editor controls. Game Master entity placement produced a matching spawn log.
- A guarded dedicated server downloaded Raven, entered Conflict Arland, and bound only to loopback. A private player-hosted Raven Conflict session reached gameplay and logistics startup. Raven delivery was not verified; the user ended that probe.
- The `workbench:open --execute` World Editor `-load` path was live-tested on GM Arland and the Convoy Follower subscene. The Arland scene has a US spawn, three driver groups, and four M923A1 trucks. The latest combined build is `.cache/local-addons/ConvoyFollower/data.pak` with pack log `.cache/workbench-runs/convoy-everon-soviet-seat-final-pack-v2`; it passed Workbench validation across five configurations and packed without addon resource errors. Earlier gameplay confirmed three-truck following and leader voice. The M923 rear **Pull off and regroup** action, sequential unloading, parked return line, and automatic homeward return crossing passed script validation and packaging, but their menu visibility and road maneuvers need the next F5 player test. The new Soviet driver group and standard Ural rear action also passed packaging, pending a Soviet in-game smoke test. The separate [Everon night demo](convoy-everon-demo.md) stages a US convoy from Regina to Levie with supplies and a lead LAV; the sports-ground vehicles and Levie storage were visually inspected in World Editor, while its route, LAV seat transfer, supply handling, and 22:00 start need a player test. See [convoy-mapmaker-quickstart.md](convoy-mapmaker-quickstart.md) for exact controls and [convoy-workshop-release.md](convoy-workshop-release.md) for remaining release checks. The `conflict:host` live branch and an untimed player handoff remain untested; dedicated-server client joining timed out even in a vanilla control.

See [operations-playbook.md](operations-playbook.md) for detailed commands, logs, UI quirks, and recovery notes. See [raven-field-notes.md](raven-field-notes.md) for the limited Raven result.
