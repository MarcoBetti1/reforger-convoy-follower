# Ordinary arrival hold: repeated physical comparison

September 26, 2026. This is a bounded production-follower result, not release acceptance.

The controller now requests native cruise zero while ordinary road/trail arrival is latched. Explicit Hold retains precedence. Resuming normal following releases the arrival latch and restores pacing. Native steering and waypoint execution are unchanged. New read-only activity diagnostics identify the selected behavior, group action, seat controls and waypoint state.

Both fresh launches used the same immutable `convoy-arrival-zero-v1` package, SHA-256 `30D094F57232348A4E891AA35FA46FF06D480A4CC20C7396CBCB605AB98C5225`, and the Arland taxiway course with a native AI lead and passenger owner. The follower used the production controller; no fixture supplied its braking or navigation. Settings were completion radius **5 m**, moving gap **20 m**, stopped gap **10 m**, native cruise enabled. Radius five remains experimental; the shipped default is still zero/legacy spacing-derived radius.

| Observation | First run | Fresh repeat |
| --- | --- | --- |
| Run folder under `.cache/client/runs/` | `arrival-zero-radius5-v1` | `arrival-zero-radius5-v2` |
| Largest independently retained per-link gap | 47.3505 m | 51.4730 m |
| First settled hold | 30 s | 30 s |
| Powered lead/follower restart | Passed | Passed |
| Second hold before terminal | 30 s | 30 s |
| Post-terminal measured observation | 180 s, both trucks | 180 s, both trucks |
| Maximum follower drift during observation | 0 m | 0 m |
| Maximum lead drift during second hold | 0.000273 m | 0.000854492 m |
| Strict full run | FAIL: 9 runtime, 64 shutdown errors | FAIL: 9 runtime, 64 shutdown errors |

The unchanged short-course peak-gap gate is 60 m. The repeat's independent 51.4730 m sample is slightly above its probe summary of 51.4681 m; retain the larger value. Both runs reached normal world cleanup and `Game destroyed`. The nine runtime signatures match saved vanilla controls but remain present in the strict result. Cleanup is observed, not error-free.

These repetitions support the arrival-hold change on this course. They do not prove every startup is smooth: the repeat briefly slowed almost to zero after the initial MOVE rebuild. The previous moving-restart candidate had drifted 3.6998 m later, beginning while cruise already requested zero; therefore these comparisons do not establish one universal cause for all drift. Two/three followers, turns, natural terrain, changed order, possession, human input, and the supply workflow remain separate requirements.

## Evidence and reproduction

Each run retains `result.md`, `strict-full-report.json`, full `logs/console.log`, deployment hashes and timing analysis. The repeat's generic harness watcher started before launch and captured `first-result.log` about 240 ms after the result, plus `observation-complete.log` about 23 ms after the observation marker. Provenance sidecars retain capture times and hashes. The first run's early loading recording is incomplete following a monitor-index failure; preserve that limitation.

The repeat's private full recording is `.cache/test-videos/arrival-zero-radius5-v2-full.mp4` (349.8 s). Its game-region review is `arrival-zero-radius5-v2-game-review.mp4`, source seconds 30–330, crop x640/y360/1280x720. Root inspected live game state and selected recorded frames; this is a diagnostic camera view, not a normal-player supply demonstration. Raw monitor recordings remain outside public Git.

Use the existing frozen deployment `.cache/standalone-arrival-zero-v1-addons`, addon `5A5FB20BD40C7C70`, world `Worlds/Tests/ConvoyFollower_Arland_ClearField_Offroad_1Truck.ent`, a new isolated profile/run directory and the settings above. Keep observing through `OFFROAD_OBSERVATION_COMPLETE` and close normally afterward. The historical world name does not make this an unpaved test: it is concrete and must be reported with `--fixture arland --surface off-network`. Never overwrite existing runs.

The separate Resume-status fixture v2 failed during test-lead seat transfer before its new status episode. The compiled v3 fixture adds an explicit lead-only parking lease and atomic hold-release/pilot-exit transition. Its tests do not supply production follower movement or prove human control. See the current validation record for that candidate's latest outcome.
