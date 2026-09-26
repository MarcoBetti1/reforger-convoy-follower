# Native cruise integration boundary

September 26, 2026. `CF_NativeCruiseControl` is connected to ordinary driving in the existing working source, and the test-only cruise writer has been removed from the taxiway follower. Stored production-cruise-v1 logs show five-configuration validation and packaging. The completed production-cruise-radius5-v2 run demonstrates production pacing and narrow fixture hold/restart checks but fails spacing at 64.3798 m; player-commanded Hold remains unverified. See the current [validation record](convoy-validation-current.md) for newer experiments and error accounting. This documentation-only reconciliation preserves that work; further implementation or live testing awaits the user's goal update and resumption.

## API and ownership

Create one helper instance per driver controller. `Request(driver, assignedTruck, maxSpeedKmh, reason)` validates the current NPC pilot seat and the native movement agent's controlled truck on every call. It accepts a speed cap in km/h, returns whether that request is owned, and never claims physical completion. `Release(reason)` resets only while that same AI pilot still controls that exact truck. `DetachForWorldCleanup()` clears references without touching native objects. Read-only getters expose requested speed, resolver, reason, and last refusal.

The helper has no timer, state machine, waypoint handling, path selection, brake/throttle/gear writes, or reference to a lead vehicle. It cannot change convoy order or turn all trucks into player followers. The controller must provide its own assigned predecessor when calculating a cap.

The demonstrated policy remains `CF_FollowSpeedPolicy.MaxSpeedKmh(originGap, stoppedGap, predecessorSpeedKmh)`, updated every 200 ms. The helper preserves the experiment's 1 km/h write tolerance and makes transitions to and from an exact zero request immediate. No route-helper integration or tuning change is included here.

## Controller integration contract

1. Update ordinary FOLLOWING/ARRIVING pacing before the controller's one-second navigation early return, at a separate 200 ms cadence. Read the immediate predecessor's current assigned truck, never a fixed world name or always the player's truck. Measure the same horizontal origin gap used in the experiment.
2. Use the same instance for explicit Hold's cruise-zero request. A speed cap alone does not establish a durable hold; the selected native hold behavior and physical brake remain separately owned. Hold intent has precedence over pacing and survives chain rewiring/reboarding.
3. Release the old cap before changing assigned truck, driver, predecessor, movement mode, dismount order, dismissal, loss/recovery mode, or controller deletion. Planned transitions must release while the known AI pilot still occupies the old truck.
4. On unexpected seat loss or player takeover, the helper refuses future requests and abandons the lease without modifying the new pilot's controller. Since the engine exposes no safe owner-token reset, the old native cap cannot be conditionally removed after ownership is lost. Correct transition ordering and logs are required; the next authorized acquisition writes its own cap.
5. During world teardown, call only `DetachForWorldCleanup()`. Do not send native control writes after session cleanup begins.
6. Disable/remove `CF_FollowCruiseProbeComponent` for the production comparison. Its independent timer otherwise writes the same native setting and invalidates the claim that production supplies pacing.

## Limits and next physical comparison

The installed `AICarMovementComponent` exposes `SetCruiseSpeed(float speedKmH)` and `ResetCruiseSpeed()`, with no getter, ownership token, or override priority. The installed Scenario Framework action also writes the cap directly. This helper cannot restore an unknown prior limit or arbitrate with another mod that writes cruise speed. Convoy must maintain one writer per truck.

The current policy assumes 2 m/s² braking and a 1.5-second allowance, uses vehicle-origin gap, and caps at 45 km/h. It does not yet account for vehicle dimensions, corner curvature, measured braking capability, or route arc distance. Reverse predecessor speed is treated as zero. These are known boundaries for the next independent comparisons, not reasons to silently change the established baseline.

First compare the production helper against the same short taxiway pack/profile with the test follower cruise component removed, preserving whichever waypoint radius the separate radius comparison supports. Check actual controls, startup/peak gap, stopped approach, explicit Hold, restart, seat changes, and native reset logs. Then repeat one/two/three-truck straight and turn fixtures. `CF_DrivenRoute` remains a separate subsequent physical adapter experiment.
