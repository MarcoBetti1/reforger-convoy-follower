# Player control follow-up — September 26

Preserved source-review findings and proposed regressions. This documentation-only reconciliation does not resume the goal or run tests. Keep the earlier frozen native-lead comparison separate from newer follower production changes; use the current validation record for their status.

## First fix: preserve explicit Hold intent

The original source review found that `CF_DriverControllerComponent.CF_SetConvoyTarget` rewired a `CF_PANEL_HOLD` unit into `CF_WAITING_FOR_PREDECESSOR`, which could resume automatically. Member removal and dismissal call this path. Unexpected reboarding restored unloading/return states but omitted panel Hold and could fall through to `StartFollowing`. `CF_IsPanelVehicleSlow` also compared signed speed rather than absolute speed.

**Implemented in working source; stored production-cruise-v1 validation/pack passed; physical regressions pending.** The server now saves a separate per-driver Hold request when the whole-convoy command is accepted. The existing approach waypoint can finish slowing the truck; Hold captures at the existing speed boundary, while follow refresh, automatic recovery, and range/stall escalation are suppressed. Predecessor changes and renumbering retain the request and keep a fully held truck in its held state. Exact assigned-seat recovery restores Hold. Explicit Resume clears both settled and still-approaching requests; dismissal, new assignment, and cleanup also clear owned intent. Speed checks now use absolute speed.

A held member remains in the chain. A downstream wait is labelled as waiting behind a held predecessor, and a member with explicit Hold intent cannot trigger automatic order-inversion rewiring or begin an unloading maneuver. This change does not add member-selection commands, native cruise writes, or a new braking strategy.

Held reboarding failures now retain membership and the assigned-truck reservation after bounded retries, repeated unexpected exits, or a wrong-seat result. The owned boarding waypoint is removed and the panel labels the reason; no further boarding order is issued while blocked. The driver can return to the exact assigned pilot seat and recover to Hold, or the owner can use the existing Stand down action. Driver death, truck destruction, and owner/session cleanup still terminate ownership deliberately. Cancelling an incomplete native GetIn activity can cause that NPC to leave its intended vehicle; this implementation removes only its owned waypoint and does not add blanket AI cancellation or evict another occupant. These recovery outcomes still require live verification.

Diagnostics expose `CF_HasPanelHoldRequest()` and `CF_IsPanelHoldReboardBlocked()` for independent fixtures. New event markers are `PANEL_HOLD_RETARGET`, `PANEL_HOLD_RESTORED`, and `PANEL_HOLD_REBOARD_BLOCKED`.

Physical regression: stop three followers, complete Hold, then remove a middle/front member in separate cases. Advance the lead while survivors retain their identities, trucks, and Hold for at least 30 seconds with no more than 2 m drift. Exercise one ordinary compartment exit/reboard and confirm the same seated Hold again. Finally, explicit Resume must produce physical progress. Native activity/seat/assignment telemetry should explain each transition.

Also cancel a still-approaching Hold with Resume and confirm no delayed Hold appears. Exercise blocked reboarding with an occupied assigned seat: preserve the player occupant and reservation, stop retries at the configured bound, verify no automatic Follow after rechain/timeout, and confirm Stand down remains available. Keep the existing native lead comparison and its frozen pack unchanged; attach any new regression probe to a separate fixture.

## Subsequent command work

- The existing panel's Hold/Resume buttons address the whole convoy; selected members currently only affect unloading actions. Add selected-member commands after durable intent is established.
- Temporary dismount is deliberately unavailable because `StandDown` releases membership/reservations. Implement retained-assignment dismount/reboard separately from dismissal.
- Existing reservations cover active, pending, return, forward-waiting, and stranded drivers. Reuse them for explicit vehicle assignment, with one server eligibility check.
- Replacing the leader currently recruits a candidate and dismisses the former leader. Promotion/reordering of existing members should preserve their trucks and reconnect the chain deliberately.
- Commands currently use reusable display identities. Add session/roster revision and request identity before exposing reordering/reassignment, so stale commands cannot silently target a different driver.

## Input evidence boundary

The panel test fixture automatically opens native `MapMenu`; it bypasses the map key. Production close uses `CloseMenuByPreset(MapMenu)` and forwards `OnMapClose` to the base implementation. Those code paths do not establish keyboard receipt or restored human driving controls.

The next input test should observe bound action names, action listeners/context state, possession, assigned pilot seat, and pre-physics controls. Establish powered forward movement and steering before opening the map, then repeat the same inputs after closing. Engine action injection can diagnose routing but must be reported separately from an actual keyboard-input pass.

Relevant source: `CF_DriverControllerComponent.c`, `CF_ConvoySession.c`, `CF_ConvoyMapPanel.c`, `CF_RadioPlayerController.c`, and `Tests/CF_PanelPilotProbeComponent.c`. The installed `ActionManager`/`InputManager` APIs are leads for instrumentation; their runtime behavior still needs verification.
