# Development reflection and next milestones

Updated 2026-09-26 UTC after the user's request to simplify and work brick by brick.

## Product we are building

A player recruits drivers, loads supplies, leads a convoy along a sensible drivable route, stops to unload, and resumes with the same assignments. Unit One follows the player. Every other unit follows its predecessor. The leader alone communicates useful status. Hold in the vehicle, dismount, and dismiss remain distinct operations.

The route is chosen by the player. The first supported driving scope is modest speeds, forward travel, ordinary turns, dirt tracks, and clear fields that fit the follower trucks. General autonomous destination planning, squeezing through narrow terrain, and elaborate automatic turnarounds are not prerequisites for proving that core loop. Unexpected obstructions should produce an honest hold/problem state.

## What slowed the recent work

- Arrival, forward parking, passing the parked line, and return merging were tested as a long sequence. Early setup failures often prevented the intended behavior from running.
- Some short tests introduced a new scripted owner pilot. Its displayed throttle did not survive to the physics step. A previous unsigned-distance gate even counted downhill coasting. That test-driver fault was separate from follower navigation.
- A field fixture was selected near vegetation and a wall; its own clearance survey rejected it before any driving occurred.
- Historical handoff documents continued to describe old builds as current. The entry instructions now point to the concise validation checkpoint.
- The existing controller already records predecessor samples, but route progress, road projection, AI waypoint lifecycle, parking, and radio are tightly mixed. More conditionals in that component make comparisons harder.

## Changes to the working method

1. **Finish only the already-packed diagnostics.** Recover the interrupted frozen v4 run, record calibration/parking/survey results, and avoid another long return sequence. A failed setup is not a navigation result.
2. **Build one independent route-guidance component.** Record actual predecessor positions, track monotonic distance along that route, interpolate a lookahead point, and preserve spacing along bends. Do not require every point to belong to the road network. Bound history and expose missing history or unavailable guidance explicitly. Test hairpins, stops, target changes, and pruning with native deterministic cases before integration.
3. **Prove one physical truck.** Use a short visible drivable course and a fixed reference route or the already working AI lead. This deliberately does not depend on reproducing human keyboard input. Compare the existing waypoint adapter with the new route guidance. If native waypoint execution remains the cause of failure, evaluate a separate steering/throttle adapter against the same route; game vehicle physics is still used.
4. **Build the chain back up.** Repeat one, two, and three followers on a straight, a turn, dirt, and a clear field; then stop, get out, get back in, and resume. Measure progress, route error, target identity, separation, intentional waiting, and recovery. No vehicle teleporting or simulated success markers.
5. **Complete the practical supply loop.** Recruit/assign, drive, Hold, manually unload, and Resume from the panel. Preserve assignments and show authoritative accepted/executing/completed/unable states. Test real menu input separately from server-command probes.
6. **Reintroduce optional maneuvers.** Forward waiting and rear regrouping need their own short physical tests. They must not block development of normal following or claim success until the truck actually parks. Untested maneuvers stay marked experimental in release documentation.
7. **Run final integration on real maps.** Arland and Everon, US and Soviet assets, one/two/three trucks, communication, player input restoration, multiplayer ownership, and a recorded supply demonstration remain release gates.

Each test must answer one stated question. Keep successful checkpoints. Change one causal variable at a time; stop a test branch that is not reaching its intended code, repair or replace its fixture, and return to product behavior. Static geometry, compilation, command acceptance, physical execution, UI usability, and multiplayer behavior are different kinds of evidence.

## First results from the reset

- The independent route helper passed 25 native geometry cases. It remains separate from production until its physical adapter is compared.
- The final manual-input calibration still failed after explicitly waking physics: controls written after the simulation step were overwritten before the next step. Stop expanding that diagnostic. The existing AI pilot has now physically driven the short comparison route.
- Isolated forward parking worked: one driver traveled 87 m and held its seat and parked position for 20 seconds. The larger return sequence remains a separate issue.
- The first physical route comparison was an unmapped concrete taxiway. It exposed a false road-recovery hold while waiting for the lead to start, a second AI startup, and a 91 m peak gap. It did not prove dirt or grass driving. The next change targets that ordinary-arrival policy, using the same course for comparison.
- The strict reporter now requires actual moving wheel contacts before claiming unpaved coverage, and rejects peak gaps above 60 m on this short course. A terminal probe marker cannot override those checks.

## Reusable deliverables

- **Convoy Follower repository:** player-facing addon, prerecorded audio, reusable route source, scenarios, diagnostic reporters, and evidence.
- **Independent Reforger Agent Harness repository:** generic launch/pack/record workflows and verified operating lessons, with no dependency on the convoy addon.
- **Driving building block:** a documented source module and native cases first. Publish a separate Workshop dependency only after a second caller uses the same interface and its physical behavior is proved. Splitting packages is not a substitute for reliable driving.

The existing [validation checkpoint](convoy-validation-current.md) remains the source of observed results. This plan describes the next work, not completed functionality.
