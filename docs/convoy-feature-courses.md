# Short feature courses

Use these small, visible scenes to diagnose one behavior before rerunning the [full driving tests](convoy-automated-smoke.md). The [edge-case matrix](convoy-edge-case-matrix.md) records actual run outcomes. World loading, order acceptance, and a clean pack are separate from a driving PASS.

## Controlled native-lead road comparison: one, two or three followers

The opt-in [one-follower world](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Paced_1Truck.ent), [two-follower world](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Paced_2Trucks.ent), and [three-follower world](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Paced_3Trucks.ent) retain the older OpenRoad geometry and production recruitment. They request a native lead cap of 25 km/h and add a departure barrier requiring every original driver to finish native boarding. The request is not a hard measured speed bound: the earlier paced-two run reached 31.9629 km/h. The fixture never writes follower controls or starts follower engines. Production engine preparation and pacing remain part of the ordinary addon. This changed fixture is distinct from older uncapped runs, whose lead could exceed the production follower's 45 km/h cap.

The subclass logs exact truck/pilot/owner/predecessor identities, movement demand, native behavior, controls and pose each second. Per-frame measurement retains peak gaps from initial roster binding, including the departure barrier. Every truck has a wheel-material observer; surface claims require moving contact evidence rather than map appearance. Counts exclude the native lead/player vehicle.

Acceptance retains at least 150 m lead and 100 m per-follower forward travel, powered samples, the original seated chain and stopped arrival, plus a **60 m maximum per-link gap** and **180 seconds of observation with no truck drifting more than 2 m**. Only the final `PACED_RESULT` follows that observation; `AUTO_RESULT` and `PACED_ROUTE_RESULT` are provisional. Runtime and shutdown errors still have separate strict gates. A fixture-controlled native Wait holds the test lead; it cannot establish player-commanded follower Hold or human driving input.

The earlier two-/three-follower source passed five-configuration validation and packaging in `.cache/workbench-runs/convoy-paced-panel-v1/`, data SHA-256 `E85212EA74C18CE20C1190666DFD1DDE75BB2C0C0260BB6F3EABC7918E6C8A41`. Its two-follower run failed with a 230.336 m peak and never entered formal observation. The newer combined candidate passed all five configurations and packaging in `.cache/workbench-runs/convoy-departure-panel-v1/`, data SHA-256 **`C657779ABA984D49894FBBD9623DBFB6AB993DE898B22347A65C9969CA627ACD`**. It includes initial-departure Wait, panel event resolution, incoming-vehicle telemetry and the one-follower variant. The new two-follower comparison is complete: physical/strict FAIL, link peaks 103.742 / 121.072 m, followed by 180.513 seconds with zero measured drift and retained identities on all trucks. Normal cleanup retained nine runtime and 64 shutdown errors. Exact outcomes belong in the [current validation record](convoy-validation-current.md). Keep older worlds, defaults and failed comparisons unchanged. Use a 540-second client safety envelope for the fixture's 480-second bound, with normal closure after terminal evidence and observation; a timer exit does not establish normal cleanup.

The compiled one-follower variant retains the same lead, first follower, course, 25 km/h request and all physical gates; it removes only the second driver/truck and changes expected counts. A narrow parent hook keeps the original one-truck smoke's unload continuation enabled by default; the paced subclass instead enters its 180-second observation at the same road-arrival gate for every count. Its unique metadata belongs to the newer combined package, not the earlier `E85212EA...` build. The shared paced probe emits read-only `PACED_DANGER_TRANSITION` / `PACED_DANGER_ENTITY` records on sampled native incoming-vehicle behavior transitions, using public installed danger/escape parameters, velocities and bounds. These POSTFRAME records identify the selected reaction's source, not its earlier event time or every native control write. Compilation is established; the new world's physical validation remains pending.

## Packed panel input comparison

The separate [OpenRoad InputPilot](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_OpenRoad_InputPilot.ent) variant disables automatic map opening and preserves the same observer and production code. Its completed `.cache/client/runs/input-pilot-mapfree-v1/` run on `C54DF7B7...` still produced no raw key/button or mapped-action response to Up/M/click/Up. All 401 post-ready samples retained the original pilot with AI off, no native menu/editor and movement/view enabled; relevant mapped actions were active. A late cursor-position change does not calibrate key/button delivery. Normal closure retains **nine runtime / 64 shutdown errors**. The map-free comparison narrows the context hypothesis; it is not an input or driving pass, and the unsuccessful OSK-close attempt does not establish an OSK-removal comparison.

The preceding diagnostic package `8C58BE9E5458F7597F86F0EB4E7F233B92EF7CA00CBEB88D372E323C3560227B` passed five-configuration validation and packaging in `.cache/workbench-runs/convoy-path-input-observer-v2/`. PanelPilot adds read-only raw mouse/key, native focus/modal/routing and binding observations with explicit limits and a 240-second world-time cutoff. The closed `.cache/client/runs/panel-raw-input-v1/` comparison recorded eight initial raw zeros and no changes, zero callbacks/orders/map closure after click/short-drag/Escape, and all 447 post-map identities retained. Normal cleanup retains **11 runtime / 62 shutdown errors**. Bindings and handlers were present; no budget was exhausted, but raw runtime sensitivity was not positively calibrated. Do not infer successful engine input or absent physical input from screenshots, zero getters or tool returns. Up/M were not delivered in that map-open run; obtain positive ordinary-input evidence before another production-panel fix.

Use [OpenRoad PanelPilot](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_OpenRoad_PanelPilot.ent) for the separate `.cache/client/runs/panel-input-events-v3/` experiment on the combined `C657779A...` package. The observer reads actual input/context/identity state and does not drive the player's truck. The panel candidate resolves only its registered buttons through the live panel root and logs at most 12 down/up/click events per instance.

Require actual Hold callback/order/state evidence, then actual Close Map and restored native input state. Only after map closure should a supported Up/M input comparison proceed. An automatically opened map, a highlighted button or successful tool return is insufficient. The observer is bounded to approximately 240 world seconds; missing events outside its coverage or after the trace budget is exhausted are inconclusive. The completed v3 run produced zero down/up/click callbacks, orders or map closure; its resolver was never exercised and Up/M were not delivered. Normal cleanup retains 11 runtime and 64 shutdown errors. This is not a demonstrated command, menu-close or powered-player-driving fix. The prior v2 run removed slot errors but did not establish those actions; preserve its 11 runtime errors and unobserved normal lifecycle separately.

## Optional native shutdown for the paced one-follower control

`Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Paced_AutoExit_1Truck.ent` copies the existing paced one-follower geometry and enables only the new default-off `m_bPacedAutoExit` test option, with its own resource identity. It requests native standalone closure 30 seconds after the real `PACED_RESULT`, preserving the 60 m gap and 180-second/2 m hold gates. It refuses Workbench, network/console sessions, stale world/component identity, and cleanup; deletion cancels the callback. Normal driver prefabs and the original worlds remain unchanged.

The first frozen `CF7B2B77...` run (`.cache/client/runs/paced-native-close-1truck-v1/`) failed driving spacing at 140.223 m, retained identities, and completed 180.363 seconds of hold with follower maximum drift 0.0116703 m. Native close then ran after 30.0164 seconds, followed by cleanup, `Game destroyed` and natural process exit before the launcher timer. This verifies that isolated shutdown path, not a driving fix, error-free lifecycle, Workbench runtime guard test or normal human input. See the [current validation record](convoy-validation-current.md) for final error classification and package provenance.

## Arrival, Hold, and Resume: one or two followers

Worlds under `Worlds/Tests`:

- `ConvoyFollower_Arland_Road81_ShortArrival_1Truck.ent`
- `ConvoyFollower_Arland_Road81_ShortArrival_2Trucks.ent`

The lead starts 60 m before a visible Arland road junction; followers start 20 m apart behind it. A test owner occupies the real driver seat. The intended sequence is to drive the lead to the bay using test-only steering and throttle, brake, wait for arrival, issue server **Hold**, then **Resume**, and drive another 65 m. That scripted lead has not achieved powered forward movement, so these courses are held. Driver/truck assignments must survive both commands once the fixture runs. This test uses no runtime vehicle repositioning and does not test a human pressing the panel buttons.

The fixed elevated camera aims to show the approach, bay, and second leg together. Visual coverage and successful physical steering remain live test gates.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_Road81_ShortArrival_1Truck.ent --authorize-local-test-scripts --execute
```

Start F5 once (or use the freshly observed Play control if F5 is ignored), confirm `GAME`, and record the viewport. After the run, use its **console.log**:

```powershell
npm run convoy:test -- --trucks 1 --variant short-arrival --report <console.log> --require-pass
```

Change the world to `2Trucks` and `--trucks 2` for the second case. The strict gate requires:

- The exact world, one live probe, and the expected recruited drivers.
- Physical lead approach of at least 45 m and each follower moving at least 35 m.
- Eight seconds of seated, mapped-road arrival with gaps no larger than 30 m.
- Authoritative Hold completion with trucks stationary and seated, followed by accepted/completed Resume.
- At least 30 m of renewed lead displacement and 20 m from every assigned follower, then a stopped road chain.
- Positive signed road progress on both lead legs, forward gear and engaged clutch samples, and at least 1 m of elevation gain on the second leg.
- No relevant runtime error, terminal stuck event, or member removal before the result.

**Current evidence:** geometry was measured in live `GAME`, and the feature-courses-v3 scripts passed all five Workbench configurations and packaging. The first frozen-v2 one-truck run failed before convoy driving: the test lead moved only 0.053 m in 75 seconds with neutral selected. The gear-corrected run `.cache/client/runs/short-arrival-1truck-v2/gameplay-at-terminal.log` also failed: 1,206 callbacks showed gear 2, clutch 1, throttle 0.28, and brake 0, but engine RPM stayed 600 and forward progress was only 0.021 m. Test-driver power/input remains unresolved; this is not a demonstrated follower pathfinding failure. The physical one-/two-truck course has not yet passed. The geometry-only survey at `.cache/workbench-gui-runs/2026-09-26T02-20-45-764Z-34040/console.log` measured nominal road width 8 m, zero centerline gap, and connected targets:

| Station | Position `<x,y,z>` |
| --- | --- |
| Lead | `<1421.87,37.0861,3044.28>` |
| Follower 1 | `<1402.61,37.0803,3038.90>` |
| Follower 2 | `<1383.29,36.9243,3033.72>` |
| Bay | `<1479.70,35.5199,3060.25>` |
| Second goal | `<1543.73,38.6607,3070.41>` |

Vehicles are placed slightly above measured terrain to settle normally. Geometry measurements do not prove collision clearance or navigation.

## Everon field: one follower

`Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent` surveys a local 60–100 m field route for grade, obstacles, and distance beyond the mapped road before giving the AI pilot a goal. The existing convoy controller drives the follower. The first frozen-v2 live run rejected all local candidates because of walls and shrubs; no drive order ran. Log: `.cache/client/runs/everon-offroad-1truck-v1/gameplay-at-terminal.log`. A clear field and physical off-road result remain pending.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Everon_Offroad_Survey_1Truck.ent --authorize-local-test-scripts --execute
npx tsx src/convoy-offroad-report.ts --log <console.log> --require-pass
```

The strict report requires both trucks to physically cover at least 40 m beyond the mapped road edge plus an 8 m buffer, with at least eight consecutive moving seconds corroborated by position samples. They must retain a seated chain, never exceed a 60 m link gap, and settle at the goal for ten seconds with a 7–30 m following gap. The default `--surface unpaved` additionally requires at least two moving samples per truck with four or more observed wheels all touching known natural-ground materials. Missing follower telemetry, mixed paved/unknown contacts, a static clear route, or a probe's own PASS marker is insufficient. This sampling gate does not certify every point along a route.

## Arland taxiway diagnostic: one follower

`Worlds/Tests/ConvoyFollower_Arland_ClearField_Offroad_1Truck.ent` is a separate physical baseline from the failed Everon survey. Live clear-field survey candidate 120 measured an eastbound 80 m route: lead `<1620,38.5938,2940>`, follower `<1600,38.0312,2940>`, goal approximately `<1700,39,2940>`. Both trucks face east (yaw 90°); authored heights use the measured terrain. The subclass reruns the existing grade, trace, occupancy and road-distance survey at the actual settled start before allowing an order.

The lead has an ordinary AI pilot and the owner sits as its passenger. The follower receives the actual production convoy order. The test does not script owner throttle, reposition either truck during gameplay, or use the new independent route helper. The elevated film camera and wheel-surface diagnostic are reused.

**Observed v1: physical completion, strict FAIL.** Despite the historical world name, video shows an airfield taxiway and lead wheel samples report concrete, with some grass contacts. The lead traveled 76.04 m and the follower 86.93 m, finishing 7.46 m apart, but the follower initially waited and the gap peaked at 91.17 m. Repeated `ROAD_UNREACHABLE` trail rejections accompanied this delay. The probe's `OFFROAD_RESULT: PASS` does not establish acceptable following or unpaved performance. The strict report also found the base `Helipad_Lights_US_01.et` load error `Unknown keyword/data 'm_bShowDebugShape'`; no clean-run claim is made.

Evidence: `.cache/client/runs/arland-clear-field-1truck-v1/gameplay-at-terminal.log` and the [paved follower-lag diagnostic clip](media/arland-paved-follower-lag-v1.mp4). Keep this as a failed comparison baseline.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_ClearField_Offroad_1Truck.ent --authorize-local-test-scripts --execute
npx tsx src/convoy-offroad-report.ts --fixture arland --surface off-network --log <console.log> --require-pass
```

The command explicitly classifies this as an off-network course, allowing paved terrain without calling it an unpaved test. It still rejects v1 for excessive separation and its load error. For a genuine field test, omit `--surface off-network` or use `--surface unpaved`; natural-material evidence from both moving trucks is then required. The reporter defaults to the Everon fixture; `--fixture arland` requires this world's evidence and cannot reuse an earlier Everon run.

## Surface diagnostics

Feature-courses-v3 includes `CF_SurfaceContactProbeComponent` on the lead in both short-arrival scenes, the Everon field scene, and `ConvoyFollower_Arland_OpenRoad_Auto_1Truck.ent`. Every five seconds during `GAME`, `TEST_WHEEL_SURFACE` records each contacting wheel's material resource, world contact position, and speed. `TEST_SURFACE_SAMPLE` records truck position and contact count. No contact or an unnamed material is inconclusive; a road-network coordinate or a grass-colored screenshot is not material evidence.

This uses the installed `VehicleWheeledSimulation` API (`WheelCount`, `WheelHasContact`, `WheelGetContactMaterial`, and `WheelGetContactPosition`) and `GameMaterial`'s inherited `GetResourceName`. The API snapshot is under `.cache/knowledge/offline-api`; the `GameMaterial` inheritance diagram confirms `GameLibMaterial`, whose inherited `BaseResourceObject` method returns the resource name. The diagnostic reads physics state only and passed shared Workbench compilation. The older frozen feature-courses-v2 build does not include it.

**Observed material:** the gear-corrected short one-truck run recorded all six lead wheels on `{1780FBB7FA2F25FC}Common/Materials/Game/asphalt.gamemat` at `<1421.88,37.1473,3044.29>`. This confirms asphalt at that road81 start, not along an untraveled route. Evidence begins at line 281 of `.cache/client/runs/short-arrival-1truck-v2/logs/console.log`. The separate open-road baseline and Everon field still need their own surface samples.

## Scripted drivetrain evidence

Installed `SCR_FuelConsumptionComponent` declares `NEUTRAL_GEAR = 1`. Bohemia's [cinematic vehicleGO example](https://github.com/BohemiaInteractive/Arma-Reforger-Samples/blob/main/SampleMod_CinematicTutorial/Scenes/Cinematic_Tutorial_default.layer) selects gear index 2 to drive. The [simulation API](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceVehicleWheeledSimulation.html) documents clutch 1 as fully engaged. The short and forward-bypass test pilots now use those values; production AI controls are unchanged.

The earlier isolated owner-drive calibration marked 24.36 m of westward, downhill movement in neutral as a PASS. That result proves neither powered forward control nor progress toward the goal. Its clip is preserved only as `.cache/test-videos/owner-drive-calibration-coasting-diagnostic.mp4`. Short-arrival and forward-rejoin strict reports now require signed progress, so accumulated motion or reversing cannot satisfy them.

The bounded v2 calibration failed all five input stages. During active physics, PRE/POST reads caught clutch 0, throttle 0 and brake 1 while POSTFRAME readbacks showed the requested values. Physics callbacks then stopped, leaving later SIMULATE stages unexercised. The v3 follow-up explicitly woke physics on each SIMULATE stage entry: callbacks resumed (1,280 total), but PRE and POST controls still differed and signed motion was only −0.228 m over 40 seconds. It failed the powered-forward/uphill gate. Logs are under `.cache/client/runs/owner-drive-calibration-v2/` and `owner-drive-calibration-v3/`. This branch is stopped; ordinary navigation tests use the physically working AI lead.

## Independent route geometry

`CF_DrivenRoute` passed all 25 native geometry cases in actual `GAME` on frozen `convoy-guidance-v1`, after five-configuration validation and packaging. It is not wired into production driving, and those cases do not prove vehicle execution. See [the module contract](convoy-driven-route-module.md) and `.cache/client/runs/driven-route-geometry-v1/gameplay-at-terminal.log`. The next useful comparison keeps the working AI lead and changes only follower guidance.

## Running clean comparisons

Keep addon scripts and worlds unchanged while Workbench is open. Stop/close it before editing and repacking. Retain failed logs and recordings with the build that produced them. For packed-client tests, use an isolated addon parent directory and an explicit addon GUID; see [the operations playbook](operations-playbook.md). Use full-map long routes, tricky turns, multiplayer, player input, and audio as separate final acceptance gates.
