# Current validation checkpoint

Updated 2026-09-26 UTC. This is the concise status; the [edge-case matrix](convoy-edge-case-matrix.md) retains earlier runs. The mod is **not yet release ready**.

## Test selection

Use the [short feature courses](convoy-feature-courses.md) first for individual controls, terrain, and parking faults. Preserve full Arland/Everon routes for final integration. A short fixture must exercise the real controller and measure movement; geometry and accepted orders have separate results.

The [development reflection](convoy-development-reset.md) now prioritizes route guidance and a simple physical following/supply loop. Long optional return sequences are deferred while that foundation is proved.

## Evidence

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
| Ordinary off-network arrival correction | Compiled in all five Workbench configurations and packed. It keeps missing road membership from forcing road recovery during ordinary following; a stopped predecessor's actual trail supplies the final goal. Cached stop state clears when the target moves or the policy returns to mapped roads. Explicit road parking/release gates remain unchanged. Same-course physical comparison is pending. |
| Independent driven-route helper | All 25 native geometry cases passed in GAME on guidance-v1, after five-configuration compile and pack. Physical adapter comparison remains pending. It is not wired into production driving. |
| UI | Prior filmed map panel accepted Hold/Resume and displayed server completion. Direct map-key opening and restored throttle/steering after close are not yet proved. |

Current live logs:

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
- Taxiway physical diagnostic: `.cache/client/runs/arland-clear-field-1truck-v1/`.
- Independent route cases: `.cache/client/runs/driven-route-geometry-v1/`.
- Rejected Everon field: `.cache/client/runs/everon-offroad-1truck-v1/`.

The client runs retain a `gameplay-at-terminal.log` snapshot and the complete `logs/console.log`. The gameplay snapshot bounds the scenario result; the complete log also contains shutdown errors. These are separate findings, and a passing snapshot must not be described as an error-free full lifecycle. See [lifecycle review](convoy-lifecycle-review.md) for the reproduced base-game shutdown diagnostics and the separate convoy teardown fix.

The older owner-drive calibration accepted distance alone. Its lead moved downhill in the opposite direction to its facing, so that clip is not proof of powered forward driving. Future scripted-owner gates require progress in the intended direction and actual gear, clutch, brake, and throttle readings.

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

These packs passed installed Workbench script validation in five configurations and resource packaging. The latest surface and peak-gap reporter changes passed all 95 TypeScript tests across 15 files and typecheck. These checks do not establish vehicle behavior.

## Repositories and release shape

The [Convoy Follower repository](https://github.com/MarcoBetti1/reforger-convoy-follower) owns addon code, audio, scenes, and gameplay evidence. The independent [Reforger Agent Harness](https://github.com/MarcoBetti1/reforger-agent-harness) owns generic launchers, Workbench workflow, and reusable operating lessons.

Keep a single Workshop addon while extracting the road math internally. A separate driving dependency requires a proven independent caller and terrain coverage first; see [the navigation plan](convoy-navigation-release-plan.md).
