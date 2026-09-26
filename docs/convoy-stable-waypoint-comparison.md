# Fixed MOVE destination comparison

**Compiled and tested; physical and strict FAIL.** Existing evidence records that the default-off candidate passed WORKBENCH, PC, XBOX, PS4 and PS5 validation and packaging, then worsened both link gaps in the controlled two-follower comparison. It is not promoted. Preserve the completed baseline in `.cache/client/runs/paced-native-path-2truck-v2/` and candidate evidence in `.cache/client/runs/paced-stable-waypoint-2truck-v1/`; the [current validation record](convoy-validation-current.md) summarizes both. The current setup updates documentation only and leaves the saved goal untouched; further implementation and testing await the user's update and resumption.

`m_bStableFollowWaypoints` is an experimental mission/server setting, **false by default** in the config, attribute and fallback defaults. To opt in for one isolated server or standalone fixture profile, add this boolean to its existing `$profile:ConvoyFollowerSettings.json` without replacing other settings:

```json
{
  "m_bStableFollowWaypoints": true
}
```

The comparison tests whether repeatedly moving a live native MOVE destination contributes to path replanning and stop/start motion. It is not a promoted navigation fix. Keeping an old goal may instead increase stopping, spacing or recovery delays.

## Exact branch

Only `MoveWaypoint` changes. After its existing control-block and own-waypoint-in-group checks, it retains the current waypoint when all of these hold:

- The option is enabled.
- State is `CF_FOLLOWING`.
- The assigned predecessor exists, its existing still timer is below the stop-detection threshold, and its actual wheeled-simulation speed exceeds 1.5 km/h.

Retention does not call `SetOrigin`, change the saved last goal, or reset waypoint age. It returns successful retention of the existing order, not application of the requested goal or physical completion. Missing/consumed waypoint recovery still uses the existing `IssueMoveWaypoint` path. Other states and a stopped/slow predecessor retain the previous update behavior. No navigation thresholds, cruise settings, explicit commands or fixture physical gates change.

`PACED_INIT` logs the effective `stable_follow_waypoints` value before scheduled movement. `FOLLOW_MOVE_RETAINED` records actual current and requested goals, saved goal, age, predecessor speed and still timer with `update_applied=false`. It is throttled to one record per five seconds and 64 records per controller lifetime, followed by an explicit limit marker. `FOLLOW_MOVE_UPDATED` continues to identify applied updates. Absence of a retention marker after the diagnostic limit is inconclusive.

## Comparison protocol retained

For any future authorized repeat, one owner handles five-configuration compilation, packaging and live runs. Use the same frozen package/course/spacing/speed/settings with only this flag changed; record the effective init value and package hashes. Require actual powered travel, original identity/seat/predecessor retention, the unchanged 60 m whole-run link-gap gate, and 180 seconds of all-truck drift observation. Compare native path/waypoint timing and explicit retained/applied/rebuilt events; accepted commands or smaller gaps alone are not success. Do not use the interrupted disk-full path run as a complete lifecycle or timing baseline. The completed comparison below is existing evidence, not a request to rerun it during setup.

## Completed comparison

Validation and packaging logs are in `.cache/workbench-runs/convoy-stable-waypoint-v1/`. Frozen `data.pak` SHA-256 is **`E765E6B4B36C08EAD4CB50F1518F4BF006BC9171750AEBFCBBEFB89A747EB432`**. The profile copied the baseline settings and enabled `m_bStableFollowWaypoints`; effective init telemetry confirms the flag and unchanged moving/stopped gaps 20/10 m, radius 5 and requested lead cruise 25 km/h. Native steering and follower fixture writes were unchanged.

| Measurement | Complete baseline | Stable waypoint candidate |
| --- | ---: | ---: |
| Unit One peak gap | 123.379 m | **169.224 m** |
| Unit Two peak gap | 88.5554 m | **131.836 m** |
| First 60 m failure after lead order | 22.935 s | **17.334 s** |
| Original uphill reverse near x1405 | Observed | Not observed in the 120 s path window |
| Formal stationary observation | 180.814 s, zero drift | 180.363 s, zero drift |
| Normal cleanup | Observed; 64 shutdown errors | Unobserved after forced termination |

The candidate crossed the old reversal location on a clean monotonic car path in forward gear with brake zero. It still failed earlier: repeated short-goal consumption caused native Idle/braking and complete stops. The first Unit One waypoint was absent at 05:07:42.863; replacement followed at 43.612, yet the truck stopped at x1369.67. Its first emitted forward-powered sample followed at 46.447, 2.835 seconds after creation. The next replacement came only 0.216 seconds after observed absence, but another stop occurred at x1415.4. Across the sampled replacements, creation-to-forward-power intervals were approximately 2.84–5.14 seconds. These are observation bounds, not direct measurements of opaque native scheduling.

Unit One produced five created, five updated and six retained markers; Unit Two produced six created, six updated and ten retained markers. Retention remained conditional: active updates occurred when predecessors slowed or stopped. This does not test globally immutable waypoints. Every unit completed 575 path reads over 120.014 seconds without raw-budget exhaustion. Waypoints were already absent before convoy cleanup reported `present=false`, consistent with native consumption; the exact native completion condition is not exposed by these observations.

Powered travel and original truck/pilot/predecessor identities passed. Bound-roster ticks 7–311 have no missing or incomplete unit sets. All three trucks then passed 180 ordered hold samples with zero measured drift; 210.133 seconds of later rounded position samples also remained stationary. These successful gates do not override either failed 60 m link gate. Moving wheel evidence again establishes a mixed mapped-road course, including dirt road and later asphalt, not an entirely unpaved or off-network route.

The full report is `.cache/client/runs/paced-stable-waypoint-2truck-v1/paced-independent-result.md`, with exact identities, errors and snapshot integrity in `paced-independent-analysis.json` and event/path timing in `native-path-comparison.json`. The terminal snapshot was captured 17 ms after the marker and is an exact prefix of the full log. Nine runtime errors match the saved vanilla resource/signature pairs and remain strict failures. Desktop activation and two Alt+F4 attempts failed before delivery; the client ended through its 540-second forced limit, with no cleanup or `Game destroyed`. Zero observed shutdown errors is not a clean lifecycle.

The private recording contains 9,183 frames / 612.266667 seconds. Root inspected cropped frames at 60, 90 and 330 seconds and observed the game viewport at departure, large-gap travel and stationary arrival; a thin OSK border remained at the top. This verifies those samples only, not complete playback or exact absence of reversing.

## Subsequent evidence and work awaiting resumption

Keep this option off by default. The saved `.cache/native-waypoint-queue-analysis.md` records installed completion/cancellation behavior and ownership concerns: native completion can cancel movement activity, and untracked queue tails would escape the controller's single-waypoint cleanup. A queue alone is not established as a continuity fix. The later [private entity-follow comparison](convoy-entity-follow-comparison.md) tested persistent native predecessor-follow activity with vehicle planning enabled against ordinary MOVE. Pack `21658B54...` improved the one-follower moving-gap result to 50.7126 m but failed arrival with 5.28124 m drift after a fixed-MOVE fallback. It remains unpromoted and does not prove driven-route fidelity. Preserve the later stopped-approach draft separately; its final review, build and live comparison remain pending.

After the user updates and resumes the goal, use the latest evidence to isolate stopped approach while preserving the observed moving leg. Preserve the original predecessor chain, useful navigable targets, one cruise writer, possession/command cleanup, unchanged 60 m gates and full 180-second hold observation. Log activity, target and waypoint identities, actual paths and completion/power timing. If testing a queued alternative, own and cancel every queued entry. Keep pacing changes separate and do not suppress native avoidance based on the failed retention result. The current setup performs documentation reconciliation only.
