# Truthful Resume status

September 26, 2026. The status contract now has a targeted physical pass on frozen `convoy-resume-seat-v3`; the full run still fails spacing and strict runtime/lifecycle gates. The earlier `convoy-moving-restart-v1` run issued no explicit panel Resume and remains separate evidence. See the [current validation record](convoy-validation-current.md). This changes session status reporting, not navigation, cruise or the existing 60 m spacing acceptance gate.

## Observed status episode

The v3 fixture corrected a test-lead seat-transfer control gap that prevented v2 from reaching the new Resume episode. Its lead-only parking lease never supplied follower control. In `.cache/client/runs/resume-status-radius5-v3/`, the new Resume remained authoritative `waiting` through 100.397 seconds of stationary observation and 104.414 seconds without a fixture status-getter call. Completion followed production evidence of 10.6313 m progress, three forward samples and two powered samples. Independent restart checks measured 29.1049 m lead and 16.2655 m follower progress.

The 30.148-second post-result window retained the convoy and measured 0.0308848 m maximum lead drift and zero follower drift. Later camera/control samples stayed stationary through approximately 429 seconds after terminal; those later samples do not retain the fixture's continuous identity assertions. The whole-course peak gap was **67.446 m**, exceeding 60 m during warmup; the smaller new-episode peak does not erase that failure. Nine runtime and 64 shutdown errors remain, with normal cleanup observed.

Package SHA-256: `E4671BAE3CC73152785CB89D810B6908AA079F4D1634A6BBE3485DA91983E893`. The original first-result snapshot was captured 234 ms after the marker; full log, independent analysis and private 691.667-second recording remain in the run/video cache. This proves one status episode, not ordinary human input, multiple members, multiplayer or release readiness. Cancellation, repeated-command, changed-assignment, possession and curved-route branches below remain required.

## Problem and contract

The saved `explicit-hold-radius5-v1` log reported `completed: convoy following` on Resume acceptance at 01:51:56.099. Independent physical restart verification arrived at 01:52:06.131. `CF_IsMovementActive()` describes a controller state; it cannot establish physical completion.

`CF_ConvoySession` now snapshots the ordered active drivers, driver entities, assigned trucks, original lead and predecessor vehicles. Its existing authoritative one-second poll evaluates Resume even when the panel is closed. The panel getter only reads the saved status. Repeated Resume requests do not reset the pending assessment.

| Status | Meaning |
| --- | --- |
| Accepted | Resume passed preflight and follow requests were accepted. No completion is inferred. |
| Waiting | The owner is outside the original lead, a predecessor is not ready, player control suspends a member, or both vehicles are stopped within the configured stopped gap plus the existing 4 m arrival allowance. A distant stationary predecessor still creates approach demand. |
| Executing | At least one pending truck has movement demand and has not yet supplied the required evidence. |
| Completed | Every original assigned truck retains its owner, driver, pilot seat and active membership, and has supplied the physical evidence below. |
| Blocked | Seat/vehicle observation failed, a member became unavailable, the controller reported LOST/STUCK, or its read-only failure reason reports lost, blocked road arrival or blocked return. This status does not itself issue an AI order. |
| Cancelled | Hold or another maneuver supersedes Resume, an assignment/order/lead changes, a driver is dismissed, or the session ends. Cancellation stays visible until another status/order replaces it; superseding orders also retain the cancellation in `PANEL_RESUME_STATE` logs. |

There is no independent Resume wall-clock timeout. Intentional waits consume no movement-failure budget; existing controller recovery/failure policy remains responsible for genuine nonprogress. Hold retains its separate existing timeout.

## Physical evidence and limits

Each truck must supply at least three samples with >=0.25 m forward travel and >=0.25 m projected travel toward its bound predecessor, >=3 m accumulated projected progress, and >=3 m displacement from the command snapshot. At least two of those samples must have engine on, forward gear >=2, throttle >0.05 and speed >=1 km/h. The direction is recalculated from the prior sampled positions each time. Samples separated by more than three seconds and jumps beyond sampled speed times elapsed time plus 2 m are excluded.

This is conservative restart evidence, not a route-following or spacing pass. A sharp turn initially away from the predecessor can delay completion; one-second samples can miss brief powered intervals. Curved-route regressions must evaluate those limits before general promotion. No production route geometry is replaced here.

Player-controlled motion contributes no evidence and clears that member's accumulated progress. After handback, the same identities must establish new AI progress. Owner exit pauses observation; owner re-entry to the same lead can continue it. A different lead cancels the original assessment.

Cancel-unload uses the same tracker for its active outbound roster; parked return trucks remain excluded and stationary. It now requires the outbound assignments to be seated and observable before accepting cancellation. Tracking is cleared during world cleanup without issuing AI orders. Read-only controller dependencies are `CF_IsPlayerControlSuspended()` and `CF_GetResumeFailureReason()`.

## Focused fixture requirements

Root owns native validation and live execution. Extend the explicit Hold fixture or create a focused variation:

1. Establish the original driver/truck identities, a settled explicit Hold and a stationary lead. For the long wait case, place the follower inside the stopped-spacing envelope through actual driving before accepting Resume; a distant follower is correctly allowed to approach and complete while the lead stays stationary.
2. Issue Resume and record an immediate `accepted` status. Keep the lead stopped for at least 100 seconds, retain positions/seat/identities, and require `waiting`, no completion and no timeout. Do not call the panel getter during most of this interval; authoritative `PANEL_RESUME_STATE` messages must still appear. Query once afterward to corroborate the saved state.
3. Start the same lead and require ordinary powered follower restart. Preserve independent >=20 m lead and >=15 m follower fixture gates. Require `PANEL_RESUME_PROGRESS` to satisfy the three forward/two powered sample requirements before `completed`. Keep full logs, a terminal snapshot, post-marker observation and strict runtime/lifecycle classification.
4. In separate branches, supersede a pending Resume with Hold; change/dismiss an assignment; and induce a known native blocked/lost state. Require cancellation or the specific blocked status and prevent stale completion. Repeat pending Resume and confirm counters are not reset.
5. Exercise cancel-unload with an outbound member and a parked return member: only the outbound member contributes to completion. Exercise owner exit/re-entry and player possession/handback; player-driven motion must never complete Resume. Follow with two/three-member and turn cases to check per-member evidence and direction conservatism.

The initial compile/live checks should target this status contract. A successful status fixture does not settle the unresolved waypoint-spacing problem or prove human panel input restoration.
