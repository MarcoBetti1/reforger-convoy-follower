# Autonomous convoy edge cases

This matrix adds repeatable scenes around the normal one-, two-, and three-truck road baseline. It distinguishes what a fixture **intends to test** from what a live F5 run has demonstrated. Use [the primary smoke guide](convoy-automated-smoke.md) for the full road probe, log parser, and recording workflow.

| Case | World | Intended observation | Present evidence |
| --- | --- | --- | --- |
| Tricky turn and gate | `Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent`, `_2Trucks.ent`, `_3Trucks.ent` | AI lead negotiates the Arland base exit; each follower stays behind its immediate predecessor without a sustained stall or order inversion. | A previous one-truck gate run passed. A prior two-truck run stalled Unit 1 near the gate; **rerun with the current build**. |
| Open road baseline | `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_{1,2,3}Truck(s).ent` | One to three linked followers complete a connected hill-road route and settle on road in order for 20 seconds. | Earlier three-truck strict gate passed; current-build rerun is required. |
| Stop, exit, reboard, resume | `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_PauseResume.ent` | A passenger owner rides while an AI pilot drives, the lead stops, the owner gets out, both drivers stay assigned and seated, the owner reboards, then the chain completes a second road leg. | **Companion probe compiled across five configurations; no live result yet.** Requires `AUTO_PAUSE_RESULT: PASS` and a separate road `AUTO_RESULT: PASS` from the same F5 run. |
| Explicit unload and return | `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent` | Unit 1 parks clear, Unit 2 reaches the bay, owner orders a regroup, and both physically return in one chain. | Separate `--variant release` gate; see primary smoke guide for latest result. |
| Forward wait | `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_ForwardWait.ent` | Unit 1 takes a distinct player-issued **Pull ahead and wait** order, stays seated, clears the unload bay, parks at a reachable forward slot, and Unit 2 advances. | **Staging scene only; forward-action companion and live result pending.** It currently shares the surveyed road approach with the rear-release fixture so those orders can be compared under similar conditions. Wider roadside clearance still needs a World Editor visual check. |
| Everon terrain staging | `Worlds/Tests/ConvoyFollower_Everon_Offroad_Auto_1Truck.ent` | Probe a field route near Regina using a lead and one follower. | Scene exists and uses the generic road-goal scanner; **it is not yet an off-road PASS test**. A terrain-specific goal and measured time away from the mapped road are still required. |

## Workbench runs

Open one world at a time, with trusted local test-script authorization, then press F5. Workbench logs append multiple F5 and editor reload sections, so use the saved `console.log` from the current GUI run with `--require-pass`.

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_Auto_2Trucks.ent --authorize-local-test-scripts --execute
npm run convoy:test -- --trucks 2 --variant harbor --report .cache/workbench-gui-runs/<run>/console.log --require-pass

npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_PauseResume.ent --authorize-local-test-scripts --execute
npm run convoy:test -- --trucks 2 --variant pause-resume --report .cache/workbench-gui-runs/<run>/console.log --require-pass
```

The pause-resume parser requires the exact pause-resume world, `AUTO_INIT: expected=2`, GAME, passing en-route metrics, a passing road arrival, and `AUTO_PAUSE_RESULT: PASS`. A successful dismount request alone cannot pass. Inspect the video for clipping, unsafe pedestrian exits, and how the trucks behave while the owner is on foot.

The separate forward-wait world will use `--trucks 2 --variant forward-wait`. Its parser requires `AUTO_FORWARD_RESULT: PASS` in addition to the normal route evidence. It cannot pass until the production order and test-only companion are wired and observed. An accepted order alone is insufficient.

The harbor-gate run should be compared against the same build on the open road. If only the harbor scene fails, capture each unit's position, current waypoint, target/predecessor, road-center distance, throttle, brake, gear, and seconds without progress around the gate; the existing probe logs these via `AUTO_UNIT_NAV`. This identifies a turn/navigation failure separately from a general inability to follow. Keep a 1/2/3 count matrix and do not average away a failing count.

The Everon off-road scene currently gives useful staging and launch coverage only. Do not report it as successful off-road pathing based on an AI road waypoint, a package build, or a short movement clip. The acceptance gate should require an explicitly surveyed terrain target, a significant period with both vehicles beyond the mapped road edge, no prolonged immobility, and visible footage confirming the surface is drivable. Add those conditions after a terrain survey; this prevents a road-hugging path from falsely passing an off-road test.
