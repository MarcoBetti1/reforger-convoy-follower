# Convoy Follower automated smoke test

This is a test harness, separate from the Arland and Everon player handoff scenes. The three `Arland_Auto` worlds use a test-only component on the lead M923. The probe seats a test player as passenger, assigns one to three real convoy followers, and gives a separate AI pilot a road waypoint. It checks sustained motion and spacing numerically; a visible gameplay review is still required for road behavior, menu use, combat, and audio. The original harbor-gate worlds remain a stress case; separate `Arland_OpenRoad_Auto` worlds stage the trucks directly on the connected hill road for a cleaner baseline.

## Prepare and launch

Run from the workspace root:

```powershell
npm run workbench -- validate --project addons/ConvoyFollower/addon.gproj --execute
npm run workbench -- pack --project addons/ConvoyFollower/addon.gproj --output .cache/local-addons/ConvoyFollower --execute
npm run convoy:test -- --trucks 1 --execute
npm run convoy:test -- --trucks 2 --execute
npm run convoy:test -- --trucks 3 --execute
```

Add `--variant open-road` to select the separate road baseline. The default `harbor` variant keeps the original harbor-gate scenes. The open-road positions came from a live road-network survey: the lead starts near `(1371,20,3330)` and followers line up westward near `(1347,18,3331)`, `(1327,15,3332)`, and `(1301,13,3334)`. All three open-road follower counts passed earlier F5 route probes. The final matrix must be rerun with the newer en-route quality gate; release and return have a separate test.

The dedicated two-truck unload and return scenario uses `--trucks 2 --variant release`. Its report only passes if both the normal sustained arrival and `AUTO_SEQUENCE_RESULT: PASS` appear. For Workbench F5, open `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent` directly. This separate fixture starts farther west on the surveyed open hill road and requests a connected stop near `(1541.6, 3337.6)`, over 200 m ahead; the normal one-, two-, and three-truck baseline worlds retain their original route scan and positions. The standalone `convoy:test --execute` client path is still unverified for automated player possession.

### Final F5 matrix

Run each world in Workbench with `--authorize-local-test-scripts`, press F5, wait for `AUTO_RESULT`, then exit the preview. Save each run's **`console.log`** path; `script.log` lacks the world-load line needed by the report gate.

| Case | World under `Worlds/Tests` | Required result |
| --- | --- | --- |
| One follower | `ConvoyFollower_Arland_OpenRoad_Auto_1Truck.ent` | Road and en-route PASS; one-truck release separately reported |
| Two followers | `ConvoyFollower_Arland_OpenRoad_Auto_2Trucks.ent` | Road and en-route PASS |
| Three followers | `ConvoyFollower_Arland_OpenRoad_Auto_3Trucks.ent` | Road and en-route PASS; video should show all four trucks |
| Unload/return | `ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent` | Road and en-route PASS **and** physical unload/return sequence PASS |

For each saved log, run the matching read-only gate (replace the paths):

```powershell
npm run convoy:test -- --trucks 1 --variant open-road --report .cache/workbench-gui-runs/<one-run>/console.log --require-pass
npm run convoy:test -- --trucks 2 --variant open-road --report .cache/workbench-gui-runs/<two-run>/console.log --require-pass
npm run convoy:test -- --trucks 3 --variant open-road --report .cache/workbench-gui-runs/<three-run>/console.log --require-pass
npm run convoy:test -- --trucks 2 --variant release --report .cache/workbench-gui-runs/<release-run>/console.log --require-pass
```

`--require-pass` exits nonzero on a failed gate. The report selects the last actual F5 `GAME` session even if exiting F5 appends another editor `AUTO_INIT`. It requires the selected world, expected truck count, GAME transition, a passing route result, and `AUTO_EN_ROUTE_METRICS` below the sustained warning/nonprogress limits. It does not prove menu usability, audio quality, combat, or visual road alignment; inspect the captured video for those.

`convoy:test` dry-runs unless `--execute` is supplied. Each run uses a fresh `.cache/convoy-tests/runs/<timestamp>-<pid>` log directory and writes `smoke-report.json`. Use `--run-dir` for a chosen fresh directory. The runner requires the packed addon under `.cache/local-addons/ConvoyFollower`; use `--addons-dir` if packing elsewhere. It launches a standalone direct-world client for 360 seconds by default. The September 26 standalone open-road 1/2/3-truck and two-truck pause/resume runs passed their strict gates; this verifies scripted actor possession and those physical sequences, not manual controls. See [the edge-case matrix](convoy-edge-case-matrix.md) for exact logs and limits. A missing `AUTO_RESULT` is a failed or incomplete gameplay test.

When using `client:run` directly, pass both `--addon 5A5FB20BD40C7C70` and the **parent** of the packed addon folder with `--addons-dir`. Use one frozen build per root, for example `.cache/test-addons/ConvoyFollower/{addon.gproj,data.pak}`, and pass `.cache/test-addons`. Addon discovery does not activate it, and passing the pack folder itself does not satisfy the launcher's child-folder GUID check.

For a visible Workbench preview, open one auto world and press F5 using the locally authorized script-test flow:

```powershell
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Arland_Auto_1Truck.ent --authorize-local-test-scripts --execute
```

Change `1Truck` to `2Trucks` or `3Trucks` for the other cases. The probe creates and possesses a US Crew character without the Game Master playable-faction menu, seats it as a passenger, boards a separate AI pilot, and calls the production `CF_ConvoySession.Start`/`Add` methods. The AI pilot receives a normal Game Master priority Move waypoint. It scans 160–300 m road goals and logs `AUTO_ROUTE_SCAN`; if none connect, its 35 m diagnostic drive ends with `FAIL long road goal unavailable`, regardless of short movement. Workbench F5 has verified sustained one-, two-, and three-truck routes under prior gate versions; use the final matrix above for current acceptance. A prior three-truck run exposed a near-arrival overtake that the revised controller and stronger probe now catch.

For the road baseline in Workbench, use `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_1Truck.ent` (or `2Trucks`/`3Trucks`) in the `--world` argument above. The world must show `AUTO_INIT expected=<count>` after F5; opening its tab is not enough if a different world remains active in Workbench.

## Expected log sequence

Search the current `console.log` for `[ConvoyFollower] AUTO_`:

1. `AUTO_INIT` confirms the component loaded in the selected world.
2. `AUTO_PLAYER`, `AUTO_SEAT`, `AUTO_PILOT`, and `AUTO_READY` confirm the test player was controlled and seated as a passenger and the separate AI pilot boarded the lead truck.
3. `AUTO_ORDER` and production `CONVOY_START_PENDING`/`CONVOY_ADD_PENDING` confirm that the real assignment code accepted each unit.
4. `AUTO_ROUTE_SCAN` records the closest road and reachable candidate road points; `AUTO_DRIVE` starts the AI lead route. `AUTO_POSITION` records each truck's position, cumulative path, and gap to its predecessor every five seconds.
5. `AUTO_ROAD_GOAL_REACHED` latches the first valid road-goal crossing after at least 150 m of lead travel. `AUTO_RESULT: PASS` requires the lead to remain within 25 m of that goal and the chain to settle in order for 20 seconds: every follower traveled at least 100 m, has a predecessor gap at most 60 m, remains within 5 m of the mapped road, and every vehicle moves at most 3 m per five-second sample. En-route quality also matters: no follower may remain over the 180 m radio-warning gap behind a moving predecessor for eight seconds, or fail to advance for 15 seconds while that predecessor moves. `AUTO_ARRIVAL_STABILITY` and `AUTO_EN_ROUTE_METRICS` record these gates. The one-truck case then attempts the real explicit unload release; `AUTO_RELEASE_RESULT` separately reports whether that truck parks in the return slot. `FAIL` contains the stage or reason.

`AUTO_RESULT: PASS` is numerical movement evidence only. A captured preview should also show that the trucks remain on the road, maintain useful gaps, and do not collide. The local camera hook closes Game Master and selects the player's third-person view; the three-truck and release worlds add a test-only aerial camera that shows the full convoy or holds the unload bay in frame. The fallback 35 m drive measures movement after a road-scan failure but can never pass. A prior weaker probe moved the lead 35 m while its follower moved only 5 m; its old `PASS` was a false positive and is superseded by these stronger checks. A multi-truck unload queue and return merge remain separate gameplay gates.

## Live evidence, September 25

The first sustained one-truck F5 run selected a reachable road point at `(1394.55, 23.33, 3312.43)`. The AI lead drove 313.2 m through the base gate and up the dirt road; the follower drove 237.3 m and closed an early 146.9 m gap to 54.7 m when the lead arrived. `AUTO_RESULT: PASS sustained road following` appeared at 01:52:39 in `.cache/workbench-gui-runs/2026-09-25T06-13-32-421Z-12108/script.log`. The camera showed the player truck driving, but its forward view did not include the follower; the log supplies the follower's movement evidence.

The first run accepted the explicit unload release at 01:52:48, but the truck moved 0 m and `UNLOAD_TURN_BLOCKED` led to `AUTO_RELEASE_RESULT: FAIL`. In the next run, a bounded fallback changed from the unreachable lateral stage to a validated road slot after 12 s. The driver stayed seated and drove 20.38 m clear of the unloading bay; `AUTO_RELEASE_RESULT: PASS road arrival and explicit release` appeared at 02:03:17 in the same Workbench log. This proves initial bay clearance, not settled parking, advancement of a second truck, or the return merge.

The first open-road one-truck F5 run reached the road goal and logged `UNLOAD_RETURN_PARKED` at 02:31:06. The truck stayed seated and settled 18.17 m from its release position. The probe was still using a 20 m displacement cutoff, so its release result would have misclassified that parked state; it now requires the driver to report a settled return slot and at least 15 m of movement. The separate `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_Auto_2Trucks_Release.ent` world adds a companion probe that orders Unit 1 to pull off, waits for parked status and Unit 2's bay advance, invokes the manual regroup order, then gives the AI pilot a homeward road waypoint. This sequence has compiled but not yet passed a live F5 test.

The first sustained two-truck run on the harbor-gate variant failed. The lead drove 340.7 m to the road goal, Unit 1 advanced 143.7 m before stalling near the gate, and Unit 2 advanced 226.4 m while trying to stay behind it. The result was a ~200 m lead-to-Unit-1 gap. This run motivated both the route-breadcrumb recovery change and a separate open-road baseline. The failure line printed `widest_gap=0` due to a probe diagnostic bug; that value did not reflect the logged positions and has been corrected.

The open-road two-truck baseline passed at 02:35:17 in `.cache/workbench-gui-runs/2026-09-25T07-33-31-616Z-7952/script.log`: lead traveled 329.8 m, Unit 1 traveled 343.3 m and finished 31.1 m behind, and Unit 2 traveled 282.2 m and finished 59.5 m behind Unit 1. They caught up after the fast AI lead stopped; the moving lead-to-Unit-1 gap had briefly reached about 210 m. This is a valid route completion test, while close formation at cruising speed still needs visual tuning. The run logged no lost, stuck, reboard, or order-rewire events.

The first open-road three-truck probe had Unit 2 briefly loop/reverse as a near-gap Move waypoint completed. In the diagnostic rerun, it recovered and all three trucks traveled over 350 m; they gathered within roughly 5–13 m near the stopped lead. Unit 2 then overtook Unit 1 and finished off the mapped road (`ORDER_INVERSION` at 02:48:49, final road-center distance 6.8 m). The old probe would have missed that late failure because the lead had touched the goal 45 seconds earlier. The arrival latch and settled-chain gate above were added to detect it, and the production controller now uses a wider moving target radius and recovery hysteresis.

The next three-truck run passed the strict arrival gate at 03:06:07 in `.cache/workbench-gui-runs/2026-09-25T08-02-03-791Z-12540/script.log`: lead path 357.1 m, final road-goal distance 9.84 m, widest predecessor gap 14.18 m, no order inversion, and 20 seconds of all vehicles settled on the road. The test-only AI pilot briefly resumed after stopping, so the probe applied a persistent brake at the goal, then the vehicles stabilized. This build validated and packed at `.cache/workbench-runs/2026-09-25T08-00-52-498Z`. The test-only film camera showed all four trucks in an aerial side view during both reruns; inspect the recorded video alongside the log for visual driving quality.

An initial two-truck release-sequence run accepted the explicit release and parked Unit 1 behind the convoy (`UNLOAD_RETURN_PARKED` at 03:11:47 in `.cache/workbench-gui-runs/2026-09-25T08-08-19-791Z-29004/script.log`). Its companion probe had released before the main 20-second road-arrival check passed, and the AI lead later overrode a once-per-second brake and drove backward into the waiting area. That test cannot establish bay advance or return. The companion now waits for the main route PASS, and the test pilot holds its brake every frame until a connected homeward waypoint is assigned; this compiled and packed at `.cache/workbench-runs/2026-09-25T08-18-44-480Z`. The corrected live release sequence is still pending.

The corrected sequence passed the main route and stable arrival at 03:21:51 in `.cache/workbench-gui-runs/2026-09-25T08-19-14-118Z-6640/script.log`. Unit 1 then parked in the return slot, Unit 2 advanced 11.26 m to the bay, the explicit manual return order was accepted, and both drivers were seated in the merged roster by 03:22:50. The return drive did **not** complete: Unit 2 entered `RETURN_TURN_BLOCKED` because neither lateral road stage passed preflight; Unit 1 received follow orders but stayed near the unload area while the AI lead drove home. The convoy lead reported lost at 03:24:39. This is a production return-path failure after a verified unload handoff. The companion now fails promptly on either driver's blocked-return state rather than waiting for its full 180-second return timer.

The next live two-truck release run passed the stricter en-route and stopped-arrival checks at 03:40:05 in `.cache/workbench-gui-runs/2026-09-25T08-36-49-743Z-6940/script.log`. The lead traveled 328.7 m, peak predecessor gap was 125.5 m, and final gaps were 8.4 m and 10.3 m. Unit 1 parked in the return slot at 03:40:28; Unit 2 moved about 6.3 m from its release position and stopped 4.5 m from the saved unload bay with the next release action available. The companion still required 10 m of advance and stayed in stage 2, so this run did not exercise the return leg. Its bay gate now requires at least 5 m of real movement, arrival within 8 m of the bay, and release readiness. The new return navigation has compiled; a live end-to-end result is pending.

Another two-truck run passed the route and settled-arrival gates at 03:45:46 in `.cache/workbench-gui-runs/2026-09-25T08-43-09-573Z-12424/script.log`, but the front truck settled 14.375 m behind the lead. It had previously logged `ARRIVAL_CLOSE`; tiny residual drift then put it just outside the 14 m release-ready cutoff while the completed-arrival hold kept it there. The companion timed out waiting for release readiness. Production now retains that readiness through a small drift after confirmed close, while still requiring a stopped target and seated driver. The fix compiled and packed at `.cache/workbench-runs/2026-09-25T08-49-16-648Z`; the next live run must verify it.

The next run in `.cache/workbench-gui-runs/2026-09-25T08-50-24-557Z-17756/script.log` passed the route, released Unit 1, and parked it in the return slot. Unit 2 moved about 3.94 m from a close starting position and stopped about 6.2 m from the saved bay with its release action ready. The companion still required 5 m of travel, so it did not issue the return order. The test-only gate now requires 3 m of movement **and** arrival within 8 m of the bay, plus Unit 1 seated and parked. This accepts a short but real pull-up without treating a stationary truck as an advance.

The following run in `.cache/workbench-gui-runs/2026-09-25T08-55-52-363Z-32352/script.log` passed the road and en-route gates at 03:58:37 and accepted Unit 1's explicit release. Unit 1 reached the open road near `(1668,3365)` but remained about 44 m from its active return-slot waypoint `(1623.57,3366.88)`. The position oscillated under a metre for over a minute despite that MOVE waypoint; production reported `UNLOAD_TURN_BLOCKED` at 04:00:38 after its bounded two-minute attempt. Unit 2 was near the bay. This is a real departure navigation failure, not the companion's 3 m bay gate. Physical return following is **not verified**. The companion now records one-second release navigation diagnostics so a new run can distinguish AI braking, gear, and road routing.

The next run in `.cache/workbench-gui-runs/2026-09-25T09-04-09-218Z-20844/script.log` passed the sustained road and stopped-arrival gate at 04:06:25: lead path 325.5 m, peak predecessor gap 105 m, and final widest gap 11.8 m. The explicit Unit 1 release was accepted at 04:06:26. Unit 1 drove toward the return slot but looped around the road edge instead of parking. Production logged `UNLOAD_TURN_BLOCKED` after two minutes at 04:08:25; the companion later logged `AUTO_SEQUENCE_RESULT: FAIL`. Unit 2 also moved far away from its waiting position during this attempted release, despite an initial `UNLOAD_QUEUE_HOLD` log. That queue movement needs its own waypoint and drivetrain evidence before assigning a cause; the companion now records `AUTO_SEQUENCE_NAV_UNIT2` once per second during stage 2. **Neither settled release parking nor physical return following passed this run.**

The next F5 run in `.cache/workbench-gui-runs/2026-09-25T09-16-47-415Z-12832/script.log` stopped before the release step because the road probe reached only 15 of the required 20 continuous settled seconds at its 180-second drive deadline. At that point the lead was 7.7 m from the road goal, both followers were ready, and the widest final gap was 14.3 m; earlier small oscillations had repeatedly reset the stability timer. The en-route warning lasted 7 seconds, just below its 8-second failure threshold. The test-only drive deadline is now 210 seconds; the 20-second continuous stability, road alignment, and en-route gates remain unchanged. This was a route-probe failure and supplies no v6 release result.

The 210-second run in `.cache/workbench-gui-runs/2026-09-25T09-22-32-823Z-7532/script.log` also failed before release. Unit 1 repeatedly maneuvered around the final bend, then stopped about 10–15 m from the mapped road center while its group waypoint completed. At 04:26:45 the lead was 12.4 m from its goal and the final predecessor gaps were short, but `stable_seconds=0` because Unit 1 was off road; the route probe correctly failed. This motivated the separate straight-road release fixture above. That fixture still needs a live F5 result; no release or return claim follows from this change.

The first straight-road release run in `.cache/workbench-gui-runs/2026-09-25T09-31-00-728Z-31496/script.log` passed the >200 m route and settled-arrival gate at 04:32:53. Unit 1 accepted the explicit release and reached `UNLOAD_RETURN_PARKED` at 04:33:34. Unit 2 was then ordered toward the stationary lead instead of the saved bay; its Move waypoint completed while it was still about 9.6 m from that bay. It had started about 9.1 m from the bay, so its long maneuver produced less than 1 m of net pull-up. The companion correctly stayed in stage 2 and did not start the return leg. This run verifies one truck parking on the straight road, **not** sequential bay access or the return drive.

The next straight-road run in `.cache/workbench-gui-runs/2026-09-25T09-40-34-857Z-5776/script.log` passed the route and en-route checks at 04:43:36, but never reached the release order. Unit 1's final stopped-road goal lay about 20 m behind the lead, and its Move completed with the truck another 8–9 m short of that goal. It remained seated and on the mapped road roughly 29 m behind the lead, outside the release-ready distance. The companion reported `FAIL front truck never became release-ready at stopped arrival` at 04:44:36. Its stopped approach needs a closer road goal or tighter completion, while the route gate alone should not be interpreted as unload readiness.

The next run in `.cache/workbench-gui-runs/2026-09-25T09-46-54-515Z-20572/script.log` passed route/arrival at 04:48:55 and accepted Unit 1's release. Unit 1 drove to a clear, mapped-road waiting area about 39 m behind its unload bay, but stopped 8.017 m from the validated waiting point. The effective parked-distance threshold was 8.0 m, so no parked event fired; it stayed seated and idle until the two-minute `UNLOAD_TURN_BLOCKED` guard. Unit 2 correctly remained held. A small parking tolerance adjustment is pending a fresh live check. The full unload queue and physical return still have **not** passed.

The following run in `.cache/workbench-gui-runs/2026-09-25T09-53-15-815Z-32376/script.log` passed route/arrival at 04:55:43, released Unit 1, and logged `UNLOAD_RETURN_PARKED` at 04:56:13. The remaining truck received `UNLOAD_BAY_APPROACH_ASSIGNED` and moved a net 5.77 m to within 6.80 m of the saved bay, but its release-ready state never became true. The companion therefore failed stage 2 at 04:58:15 with `parked=true`, `next_ready=false`; it correctly withheld the return order. This verifies physical parking and a short bay approach, but the queue readiness transition and subsequent return drive remain unverified.

Summarize an existing Workbench F5 log without launching another client:

```powershell
npm run convoy:test -- --trucks 1 --report "C:\path\to\console.log"
```

## Record the visible test

`ffmpeg` is installed on this machine. The capture helper dry-runs by default and refuses to overwrite an existing MP4. Desktop Duplication captured live Workbench frames in a short check; it records the entire selected monitor, so keep other windows off that display:

```powershell
npm run convoy:record -- --backend ddagrab --output-idx 1 --fps 8 --duration-seconds 90 --output .cache/test-videos/convoy-auto-1.mp4 --execute
```

For a named window, the older GDI mode remains available:

```powershell
npm run convoy:record -- --backend gdigrab --window-title "Arma Reforger Workbench" --duration-seconds 90 --output .cache/test-videos/convoy-window.mp4 --execute
```

The named-window GDI mode produced a stale white loading frame during live Game Master gameplay. Use Desktop Duplication for the demo and inspect the resulting MP4 before claiming video proof. The console log provides timing and position evidence alongside the video.
