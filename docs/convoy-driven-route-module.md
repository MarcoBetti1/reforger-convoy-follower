# Driven route geometry module

`CF_DrivenRoute` is an independent building block, currently **not wired into production convoy control**. Its native test probe is `CF_DrivenRouteGeometryProbeComponent`. It passed all five installed Workbench configurations and 25 native cases in `GAME` on the frozen `convoy-guidance-v1` pack. Evidence: `.cache/client/runs/driven-route-geometry-v1/gameplay-at-terminal.log`, 22:07:46. Physical adapter execution remains untested; geometry tests do not prove vehicle navigation.

## Contract

Use one instance per predecessor–follower link. The caller supplies actual predecessor positions in chronological order, a stable target/route-epoch key, and the follower's actual position. No road network, entity, waypoint, vehicle controller, or invented connector point is required.

- `Reset(targetKey, actualPredecessorPosition)` deliberately starts a new assignment or route epoch. A different key in `Record` or `Query` is rejected; it never silently changes targets.
- `Record(targetKey, position)` records samples at least 0.5 m apart in the horizontal plane. A step over 30 m latches `DISCONTINUITY`; the caller must explicitly reset after checking the missing route. Calling often enough to capture corners is the caller's responsibility.
- `Query(targetKey, followerPosition, lookAhead, spacing, maxAdvance, maxCrossTrack, result)` returns a goal, tangent, horizontal route station, remaining arc distance, cross-track error and an explicit state. `maxAdvance` should be a measured movement/time budget, not an arbitrary catch-up distance. Invalid queries do not advance progress.
- Route stations are monotonic within an epoch. Projection examines the current segment and only adjacent forward segments. Inside the current segment's final capture-distance band, a better projection on the immediately next leg permits a truck to round a corner without crossing the exact old endpoint plane. The same physical progress budget still applies. It never picks a globally closest later point, which could shortcut a hairpin or crossing.
- Lookahead and predecessor spacing are measured **along the sampled curve**, with the goal capped at recorded end minus spacing. Height is interpolated from the recorded positions. A spacing hold returns the current route position; it never requests a reverse correction.

## Startup and recovery

One point is `UNSEEDED`; it is insufficient to establish a route or gap. A follower behind the first recorded segment is `APPROACH_START`, with the first actual predecessor point as the join position, `HasArcGap=false`, and `ArcGap=-1`. This is not an arrival or zero-gap result. The caller must safely approach that start or supply an earlier genuinely recorded route; the helper does not fabricate the path from the follower to the predecessor.

Once the follower reaches the first segment within the capture corridor, guidance becomes `TRACKING` or `SPACING_HOLD`. `OFF_ROUTE` and `ADVANCE_LIMIT` are explicit failures to track, not automatic teleports or cursor skips. Sparse updates around a sharp corner can legitimately require caller-controlled reacquisition. Reversing into an earlier route section or changing travel direction requires a deliberate new epoch.

History is limited to 256 points and 600 m of retained horizontal arc. Consumed points can be removed without renumbering the absolute progress station. If pruning overtakes unconsumed follower history, `HISTORY_LOST` latches and guidance stops until an explicit reset. A stopped predecessor adds no duplicate points and the spacing result remains stable.

## Integration limits

This module measures an observed trail; it does not certify that a wider or differently loaded following vehicle can traverse it. It does not detect obstacles, preserve road ownership, steer, brake, handle collisions, compute stopping distance, recover from a blocked path, or keep an AI driver seated. Horizontal geometry alone also cannot distinguish stacked roads; an eventual adapter needs vertical/terrain checks where appropriate.

A future own-steering adapter can use the goal and route tangent while applying speed, curvature and collision constraints. A native-waypoint adapter must preserve intermediate bends: issuing a single distant MOVE may let native navigation cut a corner even when the sampled route is correct. Neither adapter is implemented here.

The native probe covers startup, straight interpolation, monotonic progress, bounded advancement, off-route rejection, stopped spacing, duplicate suppression, target changes, discontinuity, an S bend, a close hairpin, height interpolation, and both history limits. Its terminal marker is `DRIVEN_ROUTE_RESULT: PASS/FAIL`; physical acceptance remains a separate one-truck recorded-route test before two- and three-truck integration.
