# Player control handover candidate

Saved September 26, 2026. Included in the existing `convoy-moving-restart-v1` package, which passed five-configuration validation and packaging. That driving run did not exercise possession or handback, so gameplay validation of this candidate remains pending. See the [current validation record](convoy-validation-current.md) for package provenance. The restored 8 m / 3 s MOVE refresh and bounded initial gate diagnostics are preserved. This documentation-only setup does not authorize a new run.

## Installed source basis

The installed `data007.pak` resource `scripts/Game/Player/SCR_PlayerController.c` has source SHA-256 `9df2c026b6eaf8c47a4af948d4322dbfc7d6a92ed1bd1b554b9577b033f89d59`. Its `SetPossessedEntity` invokes `m_OnBeforePossess(entity)` before replication transfer, AI deactivation, and `SetControlledEntity(entity)`. `m_OnControlledEntityChanged(from, to)` runs after a change. `SetAIActivation` calls the character's `AIControlComponent.ActivateAI` / `DeactivateAI`.

`CF_ControlHandoverWatch` subscribes to those invokers for connected player controllers. Player registration and a one-second refresh cover later arrivals and controller replacement. Reset, deletion, and world cleanup remove listeners. The observer never changes player ownership or AI activation.

## Ownership policy

- Before known possession of the assigned driver/truck, release this controller's cruise and brake overrides while the original NPC still occupies its exact pilot seat. Native release functions independently check live ownership.
- Keep the member, assigned truck, chain identity, and explicit Hold request. Pause controller timers and navigation while player control is present; the roster identifies the pause. Hold/Resume, dismount, and maneuver acceptance do not claim to control a possessed unit.
- Check live player control at brake, cruise, engine preparation, waypoint creation/update/removal, and ordinary following entry points. A late observation abandons ownership flags without clearing a newly human-controlled truck's controls.
- Do not remove a boarding waypoint at takeover. Native GetIn failure can queue GetOut after the player takes over. Rewiring can update the link while deferring waypoint removal.
- Resume the saved AI state only after player control is absent, the original character's AI is active, compartment entry/exit has ended, and the assigned pilot slot is empty or held by that character. An original driver on foot can then use the existing recovery path. Explicit Hold intent remains until an accepted Resume or membership cleanup.
- If the session explicitly removes a suspended member, defer native cleanup until AI control returns; issue no GetOut. World cleanup only detaches.
- Before any deferred waypoint removal on AI return, wait for the exact owned native boarding waypoint to finish naturally. A dedicated 60-second bound starts after the player is absent and the original AI is active. Explicitly held units remain visibly blocked on timeout with Hold intent, membership, and truck reservation retained; natural completion can still recover them. Owner Dismiss remains available and uses a separate early branch without GetOut. An ordinary failed handover or an already dismissed unit relinquishes bookkeeping on timeout without failing/removing the native GetIn activity.
- Confirmed driver death, truck destruction, or disappearance ends a suspended membership without waiting for AI activation or issuing native seat/control writes. This prevents a dead possessed member from retaining its truck reservation indefinitely.

The brake lease now stores the exact driver and truck on application. A later change to `m_Truck`, driver seat, or player ownership cannot cause release to reset another pilot's brakes. Ordinary brake ownership deliberately does not depend on `IsAIActivated`, because vanilla activation/LOD changes are not proof of a new pilot. The activation check applies only when returning from suspension. Cruise ownership also rejects direct player control of the truck itself. Planned ordinary dismount already changes state and releases while the original pilot remains seated.

## Limits and required evidence

Direct native `SetControlledEntity`, initial-player assignment, or another mod's control path may bypass `m_OnBeforePossess`. A newly registered controller may also be observed after takeover. The fallback prevents further convoy writes, but deliberately does not clear a pre-existing sticky brake or cruise limit after human ownership has begun. `CONTROL_HANDOVER_LATE` reports that limitation. This candidate does not prove human throttle/steering recovery on those paths.

Native cruise/brake APIs have no ownership token covering other mods. Another writer can change the same setting between this controller's writes. The retained native waypoint/group activity also remains visible to vanilla AI while that character is possessed; suspension prevents new Convoy orders but is not a blanket native activity cancellation. Runtime checks must include possession during boarding as well as a settled Hold.

Relevant markers are `CONTROL_HANDOVER_SUSPENDED`, `CONTROL_HANDOVER_LATE`, `CONTROL_HANDOVER_AI_RETURNED`, `CONTROL_HANDOVER_DISMISS_DEFERRED`, `CONTROL_HANDOVER_BOARD_WAIT`, `CONTROL_HANDOVER_BOARD_BLOCKED`, `CONTROL_HANDOVER_TERMINAL`, `VEHICLE_BRAKE_RELEASED`, and `VEHICLE_BRAKE_ABANDONED`. Acceptance needs both normal pre-possession release and direct-control fallback observation, no follower control writes during possession, unchanged membership/Hold intent, and safe return to the original AI pilot. Possession during boarding plus rechain/Dismiss, handback timeout, and blocked-held Dismiss are separate live gates. A printed release or accepted command alone is not physical control evidence.

The bounded source review also added read-only `CF_GetResumeFailureReason()` for the session's existing lost, blocked-arrival, and blocked-return states. It introduces no new movement policy. The subsequent moving-MOVE recovery experiment is owned separately by root; this handover correction preserves that branch.
