# Pause checkpoint — 2026-09-26 UTC

**Current execution status — September 26, 2026:** the user subsequently resumed the full revised goal; saved-goal state is active (`updatedAt: 1790433506`). Earlier setup-only statements below record the completed documentation turn. Continue from current code and evidence under the latest user instructions, preserving all newer work. The previous turn made documentation progress; it did not run tests or change implementation.

**Historical checkpoint; the original results below are retained.** Later local hold/restart work and source changes are summarized in [the current validation record](convoy-validation-current.md). Preserve this snapshot, its hashes, and its original results; do not use the unrun-experiment wording below to erase newer evidence or restart from an older build. The current documentation-only setup leaves the saved goal unchanged and awaits the user's update and resumption.

**Later original-graph evidence:** the one-follower comparison has a finalized physical PASS / strict full-run FAIL. The two-follower run has closed normally after a terminal spacing FAIL at 108.378 m; final independent analysis is pending. Preserve the existing runs, staged command repair and fixture draft. See [the experiment record](convoy-original-follow-experiment.md); do not restart either run because older paragraphs below describe it as work in progress.

**Newest evidence preserved during reconciliation:** keep mod HEAD `3737f08`, harness HEAD `419dba5`, and all subsequent local work. Rear-pacing `BA45A5BA...` remains physical/strict FAIL. V6 on `5EA78318...` completed its narrow engine-action calibration, while ordinary desktop input remains unresolved. The same pack's one-follower captured-Wait control is finalized: spacing FAIL at 83.192 m, 180.18 seconds with zero hold drift, and natural closure. Both latest runs retain nine runtime and 64 shutdown errors. The first trail-guide package `CE3DEF31...` subsequently compiled and ran, failing at 267.129 m peak gap with no formal hold. Its diagnostic-only v3 successor `F134EAA1...` passed all five script configurations and packaging, then failed its live repeat at 274.773 m peak, with native reversal before OFF_ROUTE and no formal hold. Preserve both packages, their source snapshots, and the separate original-graph work in progress. No implementation or test is started by this documentation reconciliation. Use [current validation](convoy-validation-current.md) and [input calibration](convoy-input-calibration.md) for exact provenance and queued decisions. Preserve every preceding control. None of this changes the historical snapshot below or authorizes execution during setup.

Newer work includes completed radius, production-cruise, boarding-handover, explicit Hold, moving-restart and arrival-zero reports, plus control-handover/Resume-status source and fixture artifacts. Two arrival-zero comparisons passed the bounded extended physical hold but retain strict runtime/shutdown failures. Resume-status v3 passed its targeted status episode while failing whole-course spacing. The later paced-two-follower run failed with a 230.336 m peak before its 180-second observation; its completed review retains nine runtime errors and unobserved normal lifecycle after timer termination. Its compiled three-follower companion is unrun. Human input, broader driving coverage and other release gates remain unresolved. Use the current validation record for exact status; the original restart list below must not cause completed experiments to be repeated or newer source to be discarded.

At the original pause, the user requested finishing the active test, saving a stopping point, and pausing until tomorrow without automatic resumption. Later work and evidence exist and are preserved. The later documentation-only setup left the saved goal untouched. Any earlier continuation is historical; further work now awaits the user's update and resumption. Release readiness remains unfinished.

The newer preserved departure/panel package `C657779A...` passed five-configuration validation and packaging. Its completed two-follower comparison fails spacing at 121.072 m, while all trucks retain identities and hold with zero measured drift for 180.513 seconds. Normal cleanup retains nine runtime and 64 shutdown errors. Panel v3 produced no widget callbacks or command/map-close effect, retaining 11 runtime and 64 shutdown errors after normal closure; the resolver was never exercised. The one-follower world remains compiled only. These newer facts do not alter the original checkpoint below. See the current validation record and [focused courses](convoy-feature-courses.md).

**Documentation reconciliation, September 26:** [the revised brief](convoy-development-brief.md) supersedes the earlier bounded overnight product direction, with [an updated roadmap](convoy-development-reset.md). This checkpoint remains historical evidence; its hashes, results, and then-unrun experiments are preserved. The basic supply loop is an intermediate milestone, followed by complete arrival, regrouping, return, and release polish. Saving and reconciling the brief does not resume the goal or authorize implementation or tests. The separate saved explicit Hold/Resume run and newer source are covered in the current validation record; do not infer their status from this older checkpoint.

## What was learned

- Normal engine preparation reduced follower startup lag from nine to five seconds.
- Native `AICarMovementComponent.SetCruiseSpeed` resolved to the actual assigned truck and measurably reduced throttle/applied brakes on approach. The follower stopped approximately 16 m behind its lead instead of crowding it. This is a test-only component; production pacing is not integrated.
- The AI test lead independently reverses after stopping. The follower was already stationary, so do not attribute that reverse to follower contact. A one-shot cruise-zero request delayed the problem but did not prove a durable hold.
- The latest probe printed PASS after ten seconds settled, then the lead reversed 8–9 seconds later. Full video and post-terminal logs invalidate a sustained-hold claim. The strict scenario also fails the 60 m peak-gap gate (observed 70.08 m).
- The initial native MOVE can finish immediately because its 20 m completion radius equals the starting separation. The new `m_fMoveCompletionRadius` setting defaults to zero (legacy behavior). No radius-5 run has happened.
- `CF_DrivenRoute` has 25 passing native geometry cases but is not connected to physical driving yet. Preserve the player/predecessor route and the daisy chain; do not make each truck chase the player.

## Exact restart point

1. Keep the proven AI lead and the short visible taxiway fixture. Fix its persistent stopped hold independently of follower changes. Check actual native activity cancellation/ownership rather than piling on more POSTFRAME brake writes. Installed APIs expose `SCR_AIUtilityComponent.GetCurrentBehavior()` and `CancelAllGroupActivityBehaviors(...)`; no seated-pilot cancellation sequence has been verified. Require at least 30 seconds of physical hold, log controls, and fail on more than 2 m drift after settling. Keep observing after any nominal terminal marker. No new cancellation fix has been implemented.
2. Run the separate geometry-radius comparison with `{ "m_fMoveCompletionRadius": 5 }` in the isolated test profile, keeping moving gap 20, stopped gap 10, the same follower cruise experiment, and the same course. Record the effective radius before the first MOVE. Do not enable the setting by default until approach, stop, and restart are proved.
3. If native pacing and waypoint lifecycle are dependable, integrate the reusable route guidance and pacing into ordinary following with strict vehicle ownership/reset hooks. Repeat straight/turn/dirt/clear-field cases for one, two, and three trucks, then stop/exit/reboard/resume.
4. Complete the practical panel-controlled supply loop before returning to elaborate parking/U-turn work. Real map, Soviet assets, actual menu input restoration, multiplayer ownership, and a filmed supply run remain release gates.

The world used here is `Worlds/Tests/ConvoyFollower_Arland_ClearField_Offroad_1Truck.ent`. Despite its historical name, it crosses an **unmapped concrete taxiway**, not a proven grass route. Use `--fixture arland --surface off-network` with the reporter. The default unpaved gate still requires moving natural-material wheel evidence from both trucks.

## Frozen source/package

- Mod branch: `codex/release-hardening` in `reforger-convoy-follower`.
- Final saved pack: `.cache/packed/convoy-pause-checkpoint-v1`.
- `data.pak` SHA-256: `19989419D4BA45498E4D6DA7203F9614E6A2084334F0EF6357FD80D1959517AA`.
- Five-configuration validation and pack logs: `.cache/workbench-runs/2026-09-26T03-52-14-972Z/`.
- This checkpoint adds the disabled geometry-radius setting; it was compiled/packed, not launched.
- The earlier lead-hold live pack hash was `62AB8EFA25448E877B61C3A9B9B971CAD0EACA02F59743598BD39ADF85773526`.

The production changes since the preceding pushed checkpoint include one-shot engine preparation, keeping an ordinary off-network hold from being reclassified as road recovery after small lead creep, and using absolute owner speed for panel Hold preconditions. Test-only cruise pacing remains attached only to the taxiway follower. No new release claim or Workshop upload.

## Evidence retained locally

- `.cache/client/runs/arland-clear-field-cruise-v1/`: first pacing comparison, full console, terminal snapshot, strict full report.
- `.cache/client/runs/arland-clear-field-cruise-hold-v1/`: lead-hold comparison, full console, terminal snapshot, strict full report, corrected result note.
- `.cache/test-videos/arland-clear-field-cruise-v1-standalone.mp4`.
- `.cache/test-videos/arland-clear-field-cruise-hold-v1-standalone.mp4` (94.5 seconds, includes about 27 seconds after the marker).
- `.cache/test-videos/cruise-hold-v1-post-terminal.png`: visible later reverse and reduced gap.

The reporter now separates scenario/surface/runtime/lifecycle results. Neither cruise run passes overall. It retains eight runtime errors in each and the later shutdown errors; only the Helipad error has an exact independently reproduced vanilla baseline. Twenty-seven focused reporter tests and typecheck passed. Raw recordings/profiles stay out of public Git.

All Reforger clients, Workbench processes, and recorders were closed at the pause. No unattended task should restart them. Generic operating lessons are synced independently to `MarcoBetti1/reforger-agent-harness`.
