# Current validation checkpoint

**Execution status:** the documentation-only setup is complete. The subsequent explicit active-goal continuation authorizes implementation and live validation under the full revised brief. Preserve the recorded setup boundary as history; it is not a standing pause. The complete product goal remains active.

Updated 2026-09-26 UTC. This is the concise status; the [edge-case matrix](convoy-edge-case-matrix.md) retains earlier runs. The mod is **not yet release ready**.

**Completed setup instruction, September 26 (historical): documentation-only.** The complete [revised development brief](convoy-development-brief.md) supersedes the bounded overnight product direction. Its embedded resume wording and older continuation notes do not authorize implementation, launches, live tests, or activation/resumption of the saved goal in this turn. Wait for the user to update and explicitly resume the goal. Preserve newer source and all existing evidence below; this reconciliation does not revert the working tree or rerun any checks.

**Current checkpoint:** preserve the newer frozen `convoy-moving-restart-v1` pack, its existing run, and the modified working tree. Its one-follower taxiway run has a provisional physical PASS and a sampled peak gap of **58.6028 m** against the unchanged 60 m gate, but later follower creep prevents a durable stationary-hold claim. The complete strict result, lifecycle review, and repeat remain open. This candidate also contains compiled control-handover and Resume-status changes that the driving run did not exercise. The separate explicit Hold/Resume report on the earlier boarding-handover pack is complete: narrow physical PASS, strict full-run FAIL. None of these results establishes release readiness.

The [pause checkpoint](convoy-pause-2026-09-26.md) remains historical evidence. Newer local source, reports, and the native-hold experiment already exist beyond that checkpoint and are preserved. The full brief is saved byte-for-byte from the attachment (SHA-256 `A5231C6A57F11876D27A09355300B9555216E64D3F6D1EA4931BB34447583D12`).

## Newest existing evidence preserved during setup

These results and files predate this documentation reconciliation; no build or test was started for setup.

- **Frozen candidate:** `.cache/packed/convoy-moving-restart-v1/data.pak`, SHA-256 `652C4BE39007C0E9FF4F9175A3FB48D835FD3555A4A13D6B289F52AE8FFE6759`. Existing logs in `.cache/workbench-runs/convoy-moving-restart-v1/` confirm successful validation in WORKBENCH, PC, XBOX, PS4, and PS5, then successful packaging. The deployed package includes matching project and resource-database files.
- **Existing moving-restart run:** `.cache/client/runs/moving-restart-radius5-v1/`. Settings remain completion radius 5, moving gap 20, stopped gap 10, and production native cruise enabled. The saved terminal marker reports a seated chain, 117.954 m lead path, 128.236 m follower path, and 11.7298 m final gap; its observation marker records 30 seconds with `failed=false`. Independent saved timing analysis found a 58.6028 m link sample, slightly above the probe's 58.5319 m maximum. Retain the larger value and unchanged gate.
- **Startup interpretation:** first missing-MOVE reissue moved from +12.184 s in the boarding-handover baseline to +8.149 s, but that replacement also disappeared and required another request at +10.167 s. The first active update was +13.183 s. This is a promising single comparison, not proof that movement continuity or repeated starts are solved.
- **Later limitation:** the existing console at 02:19:30.429 reports follower speed 0.0876101 km/h and a 15.1242 m gap while the lead is stationary, versus 11.7298 m at the terminal. Preserve this later creep and review both trucks beyond the bounded observation window before claiming a durable hold. The strict full-run and lifecycle review is still pending.
- **Capture provenance:** `gameplay-at-terminal.log` was actually captured after the result marker, following a failed sharing-mode read. `terminal-snapshot-provenance.json` records that method; do not describe it as an exact live capture at the terminal. Full logs and the existing private recording remain separate evidence.
- **New source:** preserve the compiled [control-handover candidate](convoy-control-handover.md), [authoritative Resume-status candidate](convoy-resume-status.md), production pacing, and completed explicit-Hold reporter. No possession or explicit panel commands occurred in the moving-restart comparison, so those candidates still need their own physical validation. The newer `CF_ResumeStatusProbeComponent` and matching ResumeStatus world/layer files are unfinished fixture work; do not attribute them to this frozen pack or claim they passed validation.

After the user updates and resumes the goal, reconcile the complete existing run and later creep first, then select the next comparison and the separate control/status fixtures. Do not repeat completed historical experiments merely because the brief describes their earlier status.

## Earlier evidence preserved through setup

The existing `.cache/client/runs/native-lead-hold-v2/` log and stored report corroborate a test-owned native wait: 30 seconds of first hold, powered lead restart, follower restart, and a second 30-second hold followed by 30 more seconds of observation, all with zero measured lead drift. Signed restart progress was 24.502 m for the independent lead gate; the combined marker later recorded 39.9711 m lead and 17.8267 m follower progress. This is a narrower fixture success, **not** a production hold or release pass.

The same run still fails the unchanged 60 m following-gap gate: peak separation was **75.712 m**. The updated stored report includes nine runtime errors, adding the previously omitted GUI error. Its timed stop has no `WORLD_CLEANUP` or `Game destroyed` marker: lifecycle remains **unobserved**, not clean. Preserve the original terminal snapshot and full console.

The same-pack radius-five comparison is already complete in `.cache/client/runs/native-lead-hold-radius5-v2/result.md`. With moving gap 20 and stopped gap 10, it passed the narrow sustained-hold/restart gates but still failed following: **65.0769 m** independently observed peak gap, against the unchanged 60 m gate. First hold lasted 30 seconds; the second lasted 30 seconds plus 30 seconds of post-result observation, with 0.00274319 m maximum reported drift. Follower restart progress was 21.7137 m. Normal close reached cleanup and `Game destroyed`, while retaining **64 shutdown error lines**. No unpaved or release claim follows from this concrete-taxiway result.

The saved comparison identifies earlier follower startup with radius five, followed by native braking at the lead's old starting point and a later MOVE rebuild. It does not establish that waypoint-origin updates fail to propagate. Later direct diagnostics below found no issued origin update before that first rebuild; exact completion-versus-update eligibility ordering remains unresolved. Keep each run's exact settings and recorded timeline.

Reinspection of the saved vanilla control now matches **all nine runtime resource/signature pairs**: GUI `SCR_WidgetExportRuleRoot`, M151A2 `SlidingTrackMaterial`, two BRDM2 `Parent`, four BTR70 `Parent`, and Helipad `m_bShowDebugShape`. The line mapping is in `.cache/client/runs/native-lead-hold-v2/load-error-investigation.md`. This extends the earlier Helipad-only finding without waiving errors or proving harmlessness. The saved comparison notes report 47 focused reporter tests and typecheck passed; these were not rerun during setup.

The frozen v2 pack is `.cache/packed/convoy-native-lead-hold-v2`, SHA-256 `D17AEB665902501A629D735466FA67E3982891D532FCB24E389D1D65C8C6CA50`. The [native-hold investigation](convoy-native-hold-investigation.md) records behavior ownership and subsequent fixture safeguards absent from this pack. Both radius settings used this same package.

Newer working source contains [persistent explicit Hold intent](convoy-control-followup.md), [production native cruise integration](convoy-native-cruise-integration.md), and direct `FOLLOW_MOVE_UPDATED` diagnostics. The taxiway follower no longer has the test-only cruise writer. Stored logs under `.cache/workbench-runs/convoy-production-cruise-v1/` confirm five-configuration validation and packaging; `.cache/packed/convoy-production-cruise-v1/data.pak` hashes to `8357AACE82D0817F2A194A488D93BBF2132C2BABA641ED2E024876430B1D766B`. Production Hold/rechain/seat-recovery regressions and the independent route-helper adapter remain unverified. Preserve modified source, worlds, resource database, reporter/tests, and independent harness work; do not reset to the older quoted commit.

## Latest saved comparisons and remaining validation

Earlier comparisons are preserved alongside the new boarding-handover run after resumption. Each result applies to its exact frozen package and exercised behavior.

- **Incomplete production deployment v1:** `production-cruise-radius5-v1` omitted `resourceDatabase.rdb` and produced extra addon GUID/name, replication, and network errors. It is invalid for driving comparisons. A complete frozen deployment needs matching `addon.gproj`, `data.pak`, and `resourceDatabase.rdb`. The newer launcher preflight rejects a missing/empty database; retain that existing tooling work.
- **Production cruise v2:** `.cache/client/runs/production-cruise-radius5-v2/result.md` records matching deployment files, the ordinary production pacing writer, 30-second first hold, powered restart, and a second 30-second hold plus 30 seconds of observation. Maximum second-hold lead drift was 0.0665847 m; follower restart progress was 20.7557 m. Peak link gap **64.3798 m** still fails the unchanged 60 m gate. Nine vanilla-matched runtime errors and 64 shutdown errors remain visible. Normal close reached cleanup and `Game destroyed`. This proves a narrow production-pacing/fixture-hold result, not player-commanded Hold or release readiness.
- **Moving refresh v1:** `.cache/client/runs/moving-refresh-radius5-v1/result.md` records a complete deployment but driver-seat loss before powered travel or any target update. The missing database therefore cannot be the sole explanation for seat loss. The new refresh branch was not exercised. Nine runtime and 62 shutdown errors are retained, with observed normal cleanup.
- **Moving refresh v2:** `.cache/client/runs/moving-refresh-radius5-v2/result.md` records the same immutable candidate's completed repeat. It passes 30-second first hold, powered lead/follower restart, second hold and 30 seconds of post-result observation, with 0.114222 m maximum lead drift. The strict peak link gap is **67.396 m**; separate camera telemetry observed 67.523 m. Nine runtime and 64 shutdown errors remain. Full logs, terminal snapshot, `timing-comparison.json`, and `startup-telemetry.md` are preserved. This is not evidence that reducing the moving-target refresh distance to 2 m improves following.
- **Timing limit:** in the completed refresh repeat, waypoint disappearance is bounded between the last present sample at +5.082 s and the first absent sample at +10.166 s. Lead displacement crossed 2 m between +6.999 and +7.999 s, within that interval. Rebuild occurred at +11.182 s; the first direct update was at +14.199 s. The five-second status throttle and missing per-poll eligibility values prevent establishing the exact order of native completion and refresh eligibility. Preserve that uncertainty when selecting the next diagnostic.
- **Boarding handover v1:** `.cache/client/runs/board-handover-radius5-v1/result.md` records a matching three-file deployment of the new guard/diagnostic pack. `BOARD_HANDOVER` waited with getting_in=true until ready=true three seconds later, then issued MOVE. No seat loss occurred in this fresh start. First hold lasted 30 seconds with 0.00012207 m maximum drift; lead restart reached 25.769 m with five powered samples, follower restart 21.818 m. Second hold lasted 30 seconds plus 30 seconds after terminal, with zero drift, through fixture second 137. Peak **70.0116 m** still fails, independently corroborated by 70.0034 m from rounded paired positions. Nine runtime and 64 shutdown errors remain; normal cleanup/destroyed are logged. This is a narrow handover/fixture-hold result, not a repeatability or player-command Hold pass.
- **New eligibility evidence:** restored 8 m refresh policy logged waypoint present at +7.133 s with advance 0.729 m, absent at +8.134 s with advance 4.199 m, then absent at the first sampled advance above 8 m (+9.150 s, 10.1185 m). No initial active update was eligible in the recorded polls. Rebuild occurred at +12.184 s, gap 38.2242 m; the first eligible active refresh/update followed at +15.217 s. Exact removal time remains bounded by adjacent samples; this does not resolve the earlier 2 m candidate's counterfactual.
- **Explicit Hold independent report complete:** `.cache/client/runs/explicit-hold-radius5-v1/result.md` and `strict-full-report.json` classify a narrow physical PASS: accepted server Hold, 30.1329 seconds with zero measured drift/speed while the lead advanced 40.1952 m, retained driver/truck/roster identity, then accepted Resume and 36.3685 m follower progress with two powered samples. The strict full run FAILS with nine vanilla-matched runtime errors and 64 shutdown errors; none are waived. Cleanup and `Game destroyed` are observed. The terminal prefix was reconstructed from the complete log afterward, explicitly recorded in `terminal-snapshot-provenance.json`; it was not captured live at the marker. Automatic post-terminal physical coverage is absent. Root's separate visual observation and reviewed recording samples do not replace that gate. Fixture lead-seat transfers do not prove ordinary menu input or human throttle/steering. Resume reports completion prematurely on acceptance in this pack; the status fix is a newer source candidate. The frozen package is unchanged.

Continue from source-to-pack provenance and the boarding/waypoint ownership evidence. Retain the failed 2 m candidate without promoting it solely because it compiled. The explicit Hold evidence review is complete; rechain/seat-recovery Hold regressions, safe control ownership on player possession, ordinary input, and the physical route-helper comparison remain open.

The previously recorded complete TypeScript suite passed **157 tests in 16 files**, and typecheck passed. The explicit-Hold reporter has 27 focused tests covering physical evidence, identity, ordering, and failure classification. These were not rerun during setup. The newest driving candidate adds a moving-predecessor missing-MOVE restart opportunity, retains 8 m / 3 s active refresh and the existing fallback, and requires an owned native cruise cap. Its hypothesis and unchanged gates are in `.cache/client/runs/moving-restart-radius5-v1/experiment.md`; existing compilation and the provisional live result are recorded above.

The brief's quoted commits are historical: the recorded newer mod HEAD is `f3af488`, and the independent harness HEAD is `4985c7d`. Neither HEAD alone describes the modified mod working tree. Preserve newer source, fixture drafts, reports, and packages. No implementation, build, test, game launch, or saved-goal transition was performed for this documentation setup.

## Test selection

Use the [short feature courses](convoy-feature-courses.md) first for individual controls, terrain, and parking faults. Preserve full Arland/Everon routes for final integration. A short fixture must exercise the real controller and measure movement; geometry and accepted orders have separate results.

The [adaptive roadmap](convoy-development-reset.md) prioritizes dependable movement and a complete basic supply trip as intermediate milestones. Assignment, individual commands, recovery, arrival, regrouping, return, multiplayer, and release polish remain in scope. Sequence their tests around established foundations; deferred work is not removed from the product goal.

## Evidence

The table below retains the earlier checkpoint results. The newer native-hold result above supersedes the unresolved fixture-hold status, while its overall following failure and remaining product gates stay open.

| Check | Result and limit |
| --- | --- |
| Native road geometry | 18 cases passed in GAME, including road walking, bends, angled arrivals, and a blocked corridor. |
| One/two/three-truck road following | Strict passes exist at the preceding packed checkpoint. They do not establish all terrain or current-build regression coverage. |
| Stop, exit, reboard, resume | A prior two-truck scripted AI-lead run passed. Actual player input and map-panel focus remain separate gates. |
| Revised forward lane check | Blocked-lane gameplay gate passed on the new pack: correct refusal, driver seated, 0.033 m motion in the hold. |
| Road81 three-truck forward sequence v5 | Failed. Road arrival passed; Unit One physically drove about 89 m ahead. Its brake capture occurred near the slot, but it subsequently moved backward before parking completed. Unit Two correctly did not advance. No owner pass or resume was tested. |
| Short arrival one-truck v1 | Fixture failed before convoy driving: scripted owner lead moved only 0.053 m in 75 seconds. Its neutral gear and unsigned-motion assumption were corrected in later tests. |
| Short arrival one-truck v2 | Fixture still failed with forward gear 2 and clutch 1: 1,206 callbacks, throttle 0.28, brake 0, engine RPM 600, but only 0.021 m forward progress. All six wheels reported asphalt. The separate calibration below failed too; this scripted-owner branch and the two-truck course are held. Core navigation tests use the working AI lead. |
| Isolated forward parking v1 | Fixture never reached release readiness: stationary truck centers were 14.06 m apart, just outside the 14 m gate, with no previously driven trail. No release order was issued. The v2 fixture moved the follower start 2 m farther along the measured road; production readiness thresholds were unchanged. |
| Isolated forward parking v2 | Physical PASS on frozen v4: 87.42 m traveled, 87.27 m displacement, seated driver, 1.68 m road gap, then 20 seconds parked with zero measured drift. Actual brake/handbrake stayed applied. This does not prove the larger multi-truck release or owner-pass sequence; the parked truck was partly tree-obscured in the camera view. |
| Owner-drive calibration v2 | Strict FAIL. Three POSTFRAME throttle levels made no forward progress. Pre/post-physics samples caught clutch 0, throttle 0, brake 1 despite the writes. Physics callbacks then stopped; later SIMULATE modes never exercised their write branch. |
| Owner-drive calibration v3 | Strict FAIL after explicit ACTIVE wakes at the two SIMULATE stage entries. Physics ran for 1,280 callbacks, but PRE reads still showed throttle 0/brake 1 while POST reads showed the requested 0.5/0. Signed motion was −0.228 m over 40 seconds. Awake physics and pilot locking did not establish powered forward control. This branch is stopped. |
| Session shutdown guard | Both v3 runs logged one detached session before world destruction, with no subsequent convoy AI orders. Base-game resupply/UI shutdown errors remain. Same-process world restart and in-game member removal are separate regression gates. |
| Everon field following | Fixture survey failed: fifteen 60–100 m candidates were rejected by shrubs/wall before any convoy order. No offroad movement was tested; a clearer field is being selected. |
| Arland clear-field survey | Two 80 m corridors passed terrain/obstacle/road-distance geometry checks. The later live run revealed candidate 120 crosses an airfield taxiway. Distance from mapped roads did not establish unpaved terrain. |
| Arland taxiway following v1 | Physical completion, **strict FAIL**. Lead path 76.04 m, follower path 86.93 m, final gap 7.46 m; peak gap reached 91.17 m while the follower waited, then caught up. Video and lead wheel contacts show concrete, with some grass contacts. The original probe printed PASS, but it is not a product pass or off-road proof. The strict report rejects the base Helipad load error (`m_bShowDebugShape`); the strengthened reporter also rejects peak gap >60 m and lacks natural-surface evidence for both trucks. |
| Ordinary off-network arrival correction / taxiway v2 | All five Workbench configurations and pack passed. Live comparison removed the false road blocker (zero `ARRIVAL_ROAD_BLOCKED`) and reached `ARRIVAL_TRAIL_HOLD`; final roster correctly said holding on driven route. Startup lag remained nine seconds and peak gap 92.03 m, so strict following still failed. Final gap was 7.08 m after a very close angled stop and creep. Instrumentation showed initial MOVE consumed at 20 m, engine off; the later rebuild waited for gap >30 m, followed by native engine startup. Road parking/release gates remain unchanged. |
| Engine preparation / taxiway v3 | A single ordinary `StartEngine()` request after confirmed boarding was accepted and the engine started before the later MOVE. Follower motion began at second 13 versus 17; peak gap improved from 92.03 to 71.00 m, still outside the 60 m gate. Terminal FAIL after 180 seconds: too-close angled arrival and lead backward drift, goal gap 11.68 m, link gap 6.23 m. Drift also exposed another stale-policy road-blocking transition. Early pacing and that state transition are the next bounded fixes. |
| Native cruise experiment v1 | Six speed-policy cases passed; native movement ownership resolved to the assigned truck. Actual throttle fell and brakes applied as the requested cruise speed fell. Follower stopped about 16.4 m behind the lead. The lead then independently reversed while the follower remained stationary. Strict FAIL: peak gap 70.89 m, goal gap 18.55 m, no settled terminal result. This pacing component is attached only to the test follower. |
| Native cruise plus lead-hold experiment | Requesting cruise zero on the test lead produced a ten-second settled result: 15.66 m final gap, 5.85 m goal gap. **Not a sustained-hold pass:** video and full logs show the lead reversing again 8–9 seconds after the nominal PASS marker while the follower stayed still. Peak separation remained 70.08 m. Overall strict FAIL; preserve the post-terminal footage, not just the early marker. |
| Separate waypoint geometry setting | `m_fMoveCompletionRadius` defaults to zero, preserving the old moving-gap radius. At the older pause it had only compiled/packed. The later same-pack radius-five physical comparison above has now run and still fails spacing; it is not final tuning. |
| Independent driven-route helper | All 25 native geometry cases passed in GAME on guidance-v1, after five-configuration compile and pack. Physical adapter comparison remains pending. It is not wired into production driving. |
| UI | Prior filmed map panel accepted Hold/Resume and displayed server completion. Direct map-key opening and restored throttle/steering after close are not yet proved. |

Current live logs:

- Native hold baseline and final classification: `.cache/client/runs/native-lead-hold-v2/`.
- Same-pack radius-five comparison and timeline: `.cache/client/runs/native-lead-hold-radius5-v2/`.
- Invalid incomplete deployment: `.cache/client/runs/production-cruise-radius5-v1/`.
- Completed production integration comparison: `.cache/client/runs/production-cruise-radius5-v2/`.
- Complete-package startup seat loss: `.cache/client/runs/moving-refresh-radius5-v1/`.
- Completed moving-refresh repeat and bounded startup telemetry: `.cache/client/runs/moving-refresh-radius5-v2/`.
- Boarding-handover comparison: `.cache/client/runs/board-handover-radius5-v1/`.
- Explicit Hold/Resume run, completed strict review: `.cache/client/runs/explicit-hold-radius5-v1/`.
- Moving-restart candidate, provisional result and later creep: `.cache/client/runs/moving-restart-radius5-v1/`.
- Geometry: `.cache/workbench-gui-runs/2026-09-26T02-18-48-305Z-10812/console.log`.
- Blocked lane: `.cache/client/runs/corridor-v1-forward-blocked/`.
- Three-truck forward attempt: `.cache/client/runs/road81-v5-3truck/`.
- Short arrival: `.cache/client/runs/short-arrival-1truck-v1/`.
- Gear-corrected short arrival: `.cache/client/runs/short-arrival-1truck-v2/`.
- Isolated parking readiness failure: `.cache/client/runs/forward-slot-1truck-v1/`.
- Isolated parking physical pass: `.cache/client/runs/forward-slot-1truck-v2/`.
- Owner control calibration: `.cache/client/runs/owner-drive-calibration-v2/`.
- Explicit-wake calibration: `.cache/client/runs/owner-drive-calibration-v3/`.
- Clear-field geometry: `.cache/client/runs/arland-clear-field-survey-v1/`.
- Off-network arrival comparison: `.cache/client/runs/arland-clear-field-1truck-v2/`.
- Engine preparation comparison: `.cache/client/runs/arland-clear-field-1truck-v3/`.
- Vanilla helipad control: `.cache/client/runs/vanilla-gm-arland-helipad-baseline/`.
- Cruise pacing: `.cache/client/runs/arland-clear-field-cruise-v1/`.
- Cruise plus lead hold: `.cache/client/runs/arland-clear-field-cruise-hold-v1/`, including `corrected-result.md` and `strict-full-report.json`.
- Taxiway physical diagnostic: `.cache/client/runs/arland-clear-field-1truck-v1/`.
- Independent route cases: `.cache/client/runs/driven-route-geometry-v1/`.
- Rejected Everon field: `.cache/client/runs/everon-offroad-1truck-v1/`.

The client runs retain a `gameplay-at-terminal.log` snapshot and the complete `logs/console.log`. The gameplay snapshot bounds the scenario result; the complete log also contains shutdown errors. These are separate findings, and a passing snapshot must not be described as an error-free full lifecycle. See [lifecycle review](convoy-lifecycle-review.md) for the reproduced base-game shutdown diagnostics and the separate convoy teardown fix.

The older owner-drive calibration accepted distance alone. Its lead moved downhill in the opposite direction to its facing, so that clip is not proof of powered forward driving. Future scripted-owner gates require progress in the intended direction and actual gear, clutch, brake, and throttle readings.

The 45-second vanilla Game Master Arland control loaded only `core` and `ArmaReforger`, reached GAME at 22:36:04.447, and reproduced the exact `m_bShowDebugShape` error at offset 2307 while loading base `Helipad_Lights_US_01.et` at 22:36:04.806. This identifies that load error as independently reproduced without this addon; it remains visible in the strict report and is not a claim of an error-free game lifecycle.

[Taxiway follower-lag diagnostic clip](media/arland-paved-follower-lag-v1.mp4) preserves the failed baseline for comparison. It shows paved travel and a late catch-up, not a successful unpaved convoy run. The current terrain reporter defaults to `--surface unpaved`, requiring moving natural-material wheel samples from both trucks. `--surface off-network` labels an unmapped paved course without claiming unpaved ground; its other physical, 60 m peak-gap, and error gates still apply.

## Frozen builds

| Pack | SHA-256 of `data.pak` |
| --- | --- |
| `.cache/packed/convoy-road-corridor-v1` (blocked lane and road81 v5) | `49B2460259B48CA62F45A7CE9C69F30B51E9B46B5A18E9D87CF20FD32AD45302` |
| `.cache/packed/convoy-feature-courses-v2` (short arrival v1 and first field test) | `B55B50E3972264F7D210CB6DF6F0B67EF45E1342133AFE9EF17B208D14C43FA0` |
| `.cache/packed/convoy-feature-courses-v3` (short arrival v2 and isolated parking v1) | `7700B4B063929105D68C590B0CC6B316B736256479E04260CADEC3F3ECAE69DF` |
| `.cache/packed/convoy-feature-courses-v4` (parking v2, calibration v2, field survey) | `28ECA5B662F1D8E9020821606A7ACA1816B9E855162749D60DF58220C89CE991` |
| `.cache/packed/convoy-guidance-v1` (independent route cases and wake diagnostic) | `27D2AF85070871A0013F91FD5514C51DC949E881B91E1723B888A05303398669` |
| `.cache/packed/convoy-clear-field-v1` (taxiway physical diagnostic) | `2507A0232756A0A8F52D3701FA9CD9F9A0A0636C195DE64380DA2F660FECA756` |
| `.cache/packed/convoy-offnetwork-arrival-v1` (ordinary arrival correction) | `783B9A89D036E711E3A0E952F3CF4B808FB5C43318C22AE1924719857591A18E` |
| `.cache/packed/convoy-engine-ready-v1` (one-shot engine preparation) | `195A3E7DB0A0F68C23A334826D336A2E50BC5C0ED7516262F01C7AA1C14E6D05` |
| `.cache/packed/convoy-cruise-probe-v1` (test-only follower pacing) | `A64A1B6C391FF75E61757ADE18AB6FBEB3C324F9762FCE603A1A89D9A1DB1F5E` |
| `.cache/packed/convoy-cruise-lead-hold-v1` (test lead zero-cruise request) | `62AB8EFA25448E877B61C3A9B9B971CAD0EACA02F59743598BD39ADF85773526` |
| `.cache/packed/convoy-pause-checkpoint-v1` (geometry-radius setting remains disabled) | `19989419D4BA45498E4D6DA7203F9614E6A2084334F0EF6357FD80D1959517AA` |
| `.cache/packed/convoy-native-lead-hold-v2` (legacy/radius-five controlled comparison) | `D17AEB665902501A629D735466FA67E3982891D532FCB24E389D1D65C8C6CA50` |
| `.cache/packed/convoy-production-cruise-v1` (production pacing; completed v2 still fails spacing) | `8357AACE82D0817F2A194A488D93BBF2132C2BABA641ED2E024876430B1D766B` |
| `.cache/packed/convoy-moving-refresh-v1` (2 m candidate; startup failure and completed spacing failure) | `00FB22C5F930C8D100F7A7E8CEBD685C4B2981D6B3E001430DBB1FCDDBDB1D2A` |
| `.cache/packed/convoy-board-handover-v1` (guard observed once; spacing fails; separate explicit Hold physical PASS, strict FAIL) | `72553817EA40AF6AAC237D38CF1FB8F5719E363C358424EFA92B9FE4139BAF60` |
| `.cache/packed/convoy-moving-restart-v1` (provisional spacing improvement; later creep and full review remain open) | `652C4BE39007C0E9FF4F9175A3FB48D835FD3555A4A13D6B289F52AE8FFE6759` |

These packs passed installed Workbench script validation in five configurations and resource packaging. The earlier surface and peak-gap reporter checkpoint passed all 95 TypeScript tests across 15 files and typecheck; the later 157-test record above supersedes that tooling count. These checks do not establish vehicle behavior.

Historical reporter checkpoint: 27 focused tests and typecheck passed, separating physical scenario, surface, runtime, and lifecycle results. At that point the two older cruise runs were reported with eight runtime errors, only Helipad was classified against vanilla, and their full logs retained 64 and 62 lifecycle errors respectively. The newer nine-error classification and 47-test result above supersede that coverage statement; the later runs use different frozen packages. None of these records is an error-free full-lifecycle claim.

## Repositories and release shape

The [Convoy Follower repository](https://github.com/MarcoBetti1/reforger-convoy-follower) owns addon code, audio, scenes, and gameplay evidence. The independent [Reforger Agent Harness](https://github.com/MarcoBetti1/reforger-agent-harness) owns generic launchers, Workbench workflow, and reusable operating lessons.

Keep a single Workshop addon while extracting the road math internally. A separate driving dependency requires a proven independent caller and terrain coverage first; see [the navigation plan](convoy-navigation-release-plan.md).
