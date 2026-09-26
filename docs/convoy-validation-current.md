# Current validation checkpoint

Updated 2026-09-26 UTC. This is the concise status; the [edge-case matrix](convoy-edge-case-matrix.md) retains earlier runs. The mod is **not yet release ready**.

## Test selection

Use the [short feature courses](convoy-feature-courses.md) first for individual controls, terrain, and parking faults. Preserve full Arland/Everon routes for final integration. A short fixture must exercise the real controller and measure movement; geometry and accepted orders have separate results.

## Evidence

| Check | Result and limit |
| --- | --- |
| Native road geometry | 18 cases passed in GAME, including road walking, bends, angled arrivals, and a blocked corridor. |
| One/two/three-truck road following | Strict passes exist at the preceding packed checkpoint. They do not establish all terrain or current-build regression coverage. |
| Stop, exit, reboard, resume | A prior two-truck scripted AI-lead run passed. Actual player input and map-panel focus remain separate gates. |
| Revised forward lane check | Blocked-lane gameplay gate passed on the new pack: correct refusal, driver seated, 0.033 m motion in the hold. |
| Road81 three-truck forward sequence v5 | Failed. Road arrival passed; Unit One physically drove about 89 m ahead. Its brake capture occurred near the slot, but it subsequently moved backward before parking completed. Unit Two correctly did not advance. No owner pass or resume was tested. |
| Short arrival one-truck v1 | Fixture failed before convoy driving: scripted owner lead moved only 0.053 m in 75 seconds. Gear/clutch/forward-progress calibration is being corrected. |
| Everon field following | Fixture survey failed: fifteen 60–100 m candidates were rejected by shrubs/wall before any convoy order. No offroad movement was tested; a clearer field is being selected. |
| UI | Prior filmed map panel accepted Hold/Resume and displayed server completion. Direct map-key opening and restored throttle/steering after close are not yet proved. |

Current live logs:

- Geometry: `.cache/workbench-gui-runs/2026-09-26T02-18-48-305Z-10812/console.log`.
- Blocked lane: `.cache/client/runs/corridor-v1-forward-blocked/`.
- Three-truck forward attempt: `.cache/client/runs/road81-v5-3truck/`.
- Short arrival: `.cache/client/runs/short-arrival-1truck-v1/`.
- Rejected Everon field: `.cache/client/runs/everon-offroad-1truck-v1/`.

The client runs retain a `gameplay-at-terminal.log` snapshot and the complete `logs/console.log`. The gameplay snapshot bounds the scenario result; the complete log also contains shutdown errors. These are separate findings, and a passing snapshot must not be described as an error-free full lifecycle. See [lifecycle review](convoy-lifecycle-review.md) for the reproduced base-game shutdown diagnostics and the separate convoy teardown fix.

The older owner-drive calibration accepted distance alone. Its lead moved downhill in the opposite direction to its facing, so that clip is not proof of powered forward driving. Future scripted-owner gates require progress in the intended direction and actual gear, clutch, brake, and throttle readings.

## Frozen builds

| Pack | SHA-256 of `data.pak` |
| --- | --- |
| `.cache/packed/convoy-road-corridor-v1` (blocked lane and road81 v5) | `49B2460259B48CA62F45A7CE9C69F30B51E9B46B5A18E9D87CF20FD32AD45302` |
| `.cache/packed/convoy-feature-courses-v2` (short arrival v1 and first field test) | `B55B50E3972264F7D210CB6DF6F0B67EF45E1342133AFE9EF17B208D14C43FA0` |

Both packs passed installed Workbench script validation in five configurations and resource packaging. The workspace also passed 88 TypeScript tests and typecheck before the next diagnostic changes. These checks do not establish vehicle behavior.

## Repositories and release shape

The [Convoy Follower repository](https://github.com/MarcoBetti1/reforger-convoy-follower) owns addon code, audio, scenes, and gameplay evidence. The independent [Reforger Agent Harness](https://github.com/MarcoBetti1/reforger-agent-harness) owns generic launchers, Workbench workflow, and reusable operating lessons.

Keep a single Workshop addon while extracting the road math internally. A separate driving dependency requires a proven independent caller and terrain coverage first; see [the navigation plan](convoy-navigation-release-plan.md).
