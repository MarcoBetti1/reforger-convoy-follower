# Pause checkpoint — 2026-09-26 UTC

**Execution status:** the documentation-only setup is complete. The subsequent explicit active-goal continuation authorizes implementation and live validation under the full revised brief. Preserve the recorded setup boundary as history; it is not a standing pause. The complete product goal remains active.

**Historical checkpoint; the setup-only boundary below records the previous documentation turn.** Later local hold/restart work and source changes are summarized in [the current validation record](convoy-validation-current.md). Preserve this snapshot, its hashes, and its original results; do not use the unrun-experiment wording below to erase newer evidence or restart from an older build. Older continuation notes do not override the latest setup boundary. Wait for the user to update and explicitly resume the saved goal before further implementation or live testing.

Newer work includes completed radius, production-cruise, boarding-handover, and explicit Hold reports, plus the compiled moving-restart/control-handover/Resume-status candidate and a provisional driving comparison. Later follower creep, repeatability, human input, and other release gates remain unresolved. Use the current validation record for their exact status; the original restart list below is historical and must not cause completed experiments to be repeated or newer source to be discarded.

At the original pause, the user requested finishing the active test, saving a stopping point, and pausing until tomorrow without automatic resumption. Later work exists and is preserved. The latest request again limits this turn to documentation setup; it does not activate or resume the saved goal. This remains an unfinished release-readiness goal.

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
