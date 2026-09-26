# Convoy navigation and release plan

The user's latest request prioritizes the core following loop and smaller independent building blocks. Read the [development reflection and milestones](convoy-development-reset.md) for the current execution order; the detailed scenarios below remain final coverage targets. Do not let optional parking/return sequences or a broken scripted owner pilot block simple physical follower tests.

## Decision on splitting the mod

Keep one player-facing addon for now. Split driving **inside the source** into a small, server-owned navigation contract before considering a second Workshop dependency. The current driver controller mixes boarding, following, road targeting, stuck recovery, arrival, return parking, and radio state in one large component. Extracting a separate downloadable addon before its interface and dirt-road behavior are proven would add an installation dependency without making driving reliable. A reusable driving addon becomes worthwhile only after the same navigation component works with a non-convoy caller in an independent test scene and has a versioned API.

The convoy contract remains: Unit 1 follows the player's vehicle, each later unit follows its immediate predecessor, and only the leader reports convoy status to the owner. Navigation may change how a unit reaches a target, but it must preserve this chain and server authority.

The first safe extraction is the stateless road-polyline walk currently embedded in `CF_DriverControllerComponent.c`: pass selected road points, the lead projection, travel direction, and distance ahead; return an interpolated point or failure. Keep road selection, reachability, obstacle checks, AI waypoint issuance, server state, and diagnostics in the controller until live regressions pass. This creates a testable boundary without moving the hardest state-machine behavior into an unproven dependency.

The internal helper now also has a forward-corridor clearance calculation for testing. It measures exact road segments and the truck's short connection to the centerline, with the existing 8 m obstacle margin enforced by the controller. The reason for testing it is the road 81 v4 second release: the first truck parked and its successor reached the bay, but the approach-vector lane test rejected the same parked lead. Eighteen fixed native geometry cases cover road walking, bends, reverse direction, a diagonal bay approach, a genuinely blocked lane, and invalid roads. A live blocked-lane control and successful multi-truck sequence are still required; geometry alone cannot establish safe driving.

## Evidence before changing driving

1. Preserve the current packaged checkpoint and run one-, two-, and three-truck road baselines. Record exact world, addon pack, client or Workbench mode, GAME transition, strict terminal marker, final link gaps, target identity, road distance, waypoint state, stuck reason, and video. A test runner exiting after GAME is not a gameplay pass.
2. Reproduce one failure at a time: a tight paved turn, a mapped dirt road, a genuinely off-road segment, an out-of-order truck, a stationary predecessor, a removed middle truck, and an owner stop/exit/reboard. An off-road pass needs measured time beyond the road edge and physical movement, not a road-adjacent waypoint.
3. Compare three navigation strategies under identical starts and lead speed: the current moving-target waypoint, a predecessor-trail target, and less frequent waypoint refresh. Change one variable per run. Keep collision and road-reachability checks; a trail point is guidance, not proof that it is drivable.
4. Add failure reasons to authoritative diagnostics: intentional queue hold, blocked by predecessor, no connected route, off-road start, waypoint completed short, unexpected exit, and genuine nonprogress. Recovery must not send all trucks into the same maneuver at once.

The September 26 packed three-truck run is a useful baseline: all links reached an on-road ordered stop without a sustained stall; two trucks then parked ahead. Its full return test failed because the player-vehicle bypass was blocked. That is not evidence that the normal following algorithm needs a rewrite. The Everon dirt/off-road scene is still staging only.

## Test grounds

Keep existing Arland and Everon subscenes for final game integration. Add compact, versioned scenarios with a single purpose and fixed starts:

| Scenario | Gate |
| --- | --- |
| Straight paved road | 1/2/3 trucks keep order and finish without prolonged nonprogress. |
| Tight turn | Trailing trucks negotiate the same turn without shortcutting into an obstacle or reversing after a transient order inversion. |
| Dirt road | The route follows a fixture whose road surface is verified as dirt; log the fixture surface label, tire positions, speed, road distance, and goal progress. `BaseRoad` exposes width and points, not a surface classification. |
| Off-road field | Vehicles traverse a surveyed drivable area beyond the mapped road edge for a measured interval. |
| Stop and restart | Owner stops, exits, reboards, and resumes without losing assignments; hold and dismount remain distinct. |
| Middle-unit failure | Downstream target reconnects to the next surviving predecessor and reports one useful leader call. |
| Arrival and return | Two explicit forward parks, player lead physically passes, then an explicit resume moves the original three-unit chain. A narrow rear turn refuses safely. |

First build these as small subscenes on verified road/terrain patches so they run through the same game mode and AI stack as the addon. In parallel, prototype an independent base-world driving course only if its road network, terrain collision, and vehicle navigation can be validated in Workbench. Do not replace real-map integration tests with a synthetic course. Bohemia documents [new base worlds and terrain](https://community.bistudio.com/wiki/Arma_Reforger%3AWorld_Editor%3A_Terrain_Preparation_Tutorial), [road generation](https://community.bistudio.com/wiki/Arma_Reforger%3AWorld_Editor%3A_Road_Generator), and the [road network API](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceRoadNetworkManager.html); the installed Workbench build is the authority for what actually runs here.

The first fast arrival fixture is a separate one- and two-truck subscene on Arland road 81. It starts the lead 60 m before the stop, with followers at fixed 20 m gaps, and frames both short legs with a fixed camera. A scripted owner physically steers and brakes the lead; no truck is teleported. This scene checks only arrival, authoritative Hold in vehicle, Resume, and renewed movement by every assigned truck. It does not issue a pull-off order or claim to test keyboard/menu input. The native geometry survey measured all five starts/goals on the connected 8 m road, with zero centerline offset, at `.cache/workbench-gui-runs/2026-09-26T02-20-45-764Z-34040/console.log`; driving acceptance is separate.

Keep the existing road 81 three-truck sequence as the longer forward-parking integration test. In its September 26 v2 run, the three-truck approach passed, then a test-only lead teleport roughly 25 m to the shoulder triggered `ARRIVAL_RESUMED` before a release order. Later fixtures stage the lead before follower arrival. A short arrival pass must never be presented as evidence for this separate shoulder, release, parking, owner passing, and resume sequence.

The isolated Everon offroad fixture surveys a 60–100 m field route before starting its test pilot. It requires both vehicles to travel at least 40 m beyond the mapped road edge plus 8 m and records consecutive physical movement there. A rejected terrain/obstacle survey is a fixture-selection result, not a convoy failure. A surveyed route alone is not an offroad driving pass.

A standalone base-world course is a later research spike because it must also provide an [AI world and navigation mesh](https://community.bistudio.com/wiki/Arma_Reforger%3ANew_Terrain_Setup) plus a game mode and factions. First prove a minimal straight road and one truck in that world; add curves, dirt, obstacles, and convoy behaviors only after AI pathing works there. This avoids mistaking a missing world system for a navigation-code failure.

The existing Arland OpenRoad fixtures are a provisional dirt-road candidate. Their starts near `(1371,3330)`, `(1347,3331)`, and `(1327,3332)` are road-centered, and the route is connected to a goal near `(1530,3334)`; footage shows a pale two-rut surface. Before reusing their 1/2/3 passing runs as dirt-road evidence, inspect the underlying Road entity and material in Workbench, or verify the vehicle contact material with an installed API. A two-rut appearance alone is insufficient. If it is dirt, retain that real-map fixture and add a narrower 150 m one- and two-truck dirt gate with clear tire-level video; build a synthetic base-world course later only if a controlled comparison is still needed.

## Fast, honest test loop

1. Validate scripts and pack once. Give each run a new log and video path; record the pack hash and world name.
2. Use a frozen packed client for unattended, scripted road tests while source changes are prepared. Do not overwrite its `data.pak` while it runs.
3. Use Workbench F5 for source-only and player-panel tests. Enter via the prepared world, require GAME and the scenario's terminal marker, and close the editor before changing addon source to avoid hot-reload leaks.
4. Parse physical and authoritative states separately. A command is accepted only when server validation passes; completion needs measured movement/seat/spacing and a settled state. Keep videos for visual claims and logs for state claims.
5. Run a small regression matrix after each navigation change: 1/2/3 straight, tight turn, stop/restart, selected dirt case, forward park. Promote a change only when it fixes the target failure without regressing earlier passes. Finish with US and Soviet map-maker scenes, UI input restoration, voice, and a real multiplayer owner check.

The road23 fixture showed why fixture assumptions need an early check: after a full three-truck approach passed, the shoulder stage missed an arbitrary 25 m proximity cutoff by 0.294 m and no convoy command ran. Record surveyed start/stop/shoulder distances up front, give tolerance to measurements that can drift as AI settles, and still enforce terrain, road connectivity, vehicle occupancy, and obstacle checks when the maneuver actually starts.

## Release and repository gates

Publish the harness and its machine-specific lessons in its independent GitHub repository. Keep mod source, assets, test scenes, strict results, and proof clips in the mod repository. Sync both after each validated checkpoint. Do not claim Workshop release readiness until the required end-to-end scenarios, owner panel, audio settings, multiplayer authority, vehicle coverage, and rights/metadata checks pass. If the driving layer later meets the independent-caller gate, move it into its own versioned addon/repository with a migration path; otherwise retain it as an internal module.
