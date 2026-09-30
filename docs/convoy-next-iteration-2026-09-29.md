# Convoy Follower: next iteration, September 29, 2026

## Outcome and execution boundary

Make the whole player-led supply journey materially better: prepare, recruit and load; drive with dependable AI followers and useful commands/feedback; arrive and unload trucks sequentially; park clear, regroup, and return to the original staging area with the same drivers, trucks and assignments. At least one AI follower must carry real supplies through the complete journey, then extend the unloading queue and driving comparisons to two and three followers. **AI driving quality is central to this outcome, alongside the controls and delivery workflow.** A menu-only change or outward delivery followed by a few metres onward is insufficient.

**Execution resumed later September 29:** the human explicitly authorized starting the full development goal and session after this documentation turn. The active goal now authorizes implementation and local validation against this complete brief. Preserve the original design and review evidence. The following boundary records the earlier completed documentation phase and no longer restricts the active development session.

**Earlier documentation phase:** context gathering and goal design only. This brief and the copied recording review were the only new work authorized in that phase. Addon edits and live tests were deferred to the subsequently authorized session.

Keep the current addon and tested work as the base. Both checkouts inspected at `27e3068` contain the gameplay integration from `143e091`; the managed worktree and saved project have matching addon content after CRLF/LF normalization, and identical resource database hashes. Preserve the pre-existing `.gitattributes` change and private retained-arrival OFF_ROUTE source archive. They are not newly validated production work. Production inheritance includes classes under `Scripts/Game/Tests`; that directory name is not grounds to remove them.

Driver spawning/acquisition and WCS-specific integration are deferred. Preserve existing recruitment/boarding, server ownership, the player-lead -> Unit One -> Unit Two predecessor chain, and the valued leader voice. The player drives their own lead and leads the return; autonomous control of that vehicle or navigation to a stored base is not required. Local evidence must not be called multiplayer readiness.

## Established evidence and undiagnosed problems

Read [the copied playtest review](convoy-playtest-2026-09-29.md) and [current validation](convoy-validation-current.md) before implementing. Dated brief/roadmap instructions and frozen experiments remain references, not automatic instructions to rerun old work.

| Evidence | What it establishes and what remains open |
| --- | --- |
| September 29 human playtest, 1:23-1:37 | Recruitment/boarding is described as quick and seamless. Preserve this useful beginning rather than replacing it unnecessarily. |
| Playtest, 1:59-2:33 and later travel | The user values leader voice. Ordinary human walking/driving and menu interaction are present; earlier failed automation is not proof human input is blocked. The redesigned menu still needs its own full control check. |
| Playtest, 3:09-3:56 and 7:07 | The user reports a second seated/registered driver not coming along; the later panel shows Unit Two route blocked. Cause is undiagnosed. Do not infer a recruitment defect, hill-physics problem, or the exact historical OFF_ROUTE failure without the corresponding logs and identities. |
| Playtest, 4:36-6:18 | The user likes the follower staying on the road when the player deviates. Slow movement, large gaps and turn trouble require waiting. This does not establish robust recovery, acceptable spacing or a general offroad capability. |
| Playtest, 7:36-8:17 | Commands are difficult to find and understand; internal truck names, unavailable gaps and idle pace do not help. The user explicitly distinguishes menu confusion from proof that functions fail. |
| Playtest, 8:17-8:40 | Desired queue: player drives into the bay, unloads manually and drives clear; AI waits seated behind, then the owner commands the next truck into the vacated bay. High confidence the first cleared vehicle is player-driven; its exact identity/cargo in this illustration is not established. |
| Historical one-follower native supply run | Actual 100-supply load/delivery, about 321 m powered following, 45.8258 m peak gap, 180.413 s arrival, real Hold and fresh powered Resume with the same assignment. Physical PASS, strict lifecycle FAIL with 9 gameplay/59 shutdown errors. It did not complete safe pull-off or return home. |
| Latest two-truck delivery/restart experiment | Delivered 200, held arrival for 180.296 s without measured drift and drove about 50 m onward. Formal FAIL remains: 61.0379 m head spacing and missing continuous tail restart qualification. Private retrace geometry success did not exercise the live branch. |
| Canonical `143e091` three-follower regression | Powered travel occurred, but head/Unit Three spacing failed and retained-route OFF_ROUTE canceled healthy Unit Two native parking. No sustained arrival observation followed; 9 gameplay/59 shutdown errors remain. This is a separate diagnosed regression, not the established cause of the September 29 report. |

The recording review sampled visuals and used locally recognized audio, with uncertain wording disclosed. Loaded UI values prove a loaded state, not delivery conservation. The video does not establish completed unloading, safe regroup/return or a clean lifecycle. No transcript or media processing needs to be repeated for this brief.

## Recommended controls and truthful feedback

Use a small standalone convoy radial/menu, opened by a configurable action, with a native **Convoy commands** contextual opener for discoverability. The installed `SCR_RadialMenu`, controller/input settings and selection-entry APIs provide a practical basis. Verify implementation against the currently installed signatures, rather than assuming the older offline cache is identical. [Official radial API](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__RadialMenu.html), [controller inputs](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__RadialMenuControllerInputs.html).

| Alternative | Tradeoff |
| --- | --- |
| Inventory command item opening the same menu | Worth considering as an optional opener. Item ownership, loss, quickslot and seated use add work; possession must never grant convoy ownership. It is a suggestion, not a required dependency. |
| Native contextual commands only | Familiar at the truck/driver, but current rear actions have short reach and do not solve access from the lead seat. Reuse them as an opener/fallback, avoiding divergent command logic. |
| Configurable standalone radial/menu | Recommended initial approach for on-foot and seated access. Requires discoverable input hints, conflict checks, cancellation and restored driving input. API availability alone does not prove usability. |
| Extension of stock commanding menu | A possible alternative if a contained prototype works better; shared command/group/config integration is a larger compatibility surface. |

Keep the root vocabulary short and task-oriented: Hold convoy seated, Next truck to bay, Park this truck clear, Regroup/follow, with eligible recovery actions nearby. These are proposed player meanings, not a mandate to implement every label or add a general command system. Define convoy-wide versus selected/next-truck scope visibly. Use readable unit/driver/vehicle labels; show the intended recipient before dispatch, the resulting movement, and the reason an unavailable action cannot run.

Preserve `CF_RadioPlayerController`'s validated reliable server command path and stable selected identity. Decouple roster/order feedback from its current `SCR_MapEntity` receiver. Parked-line Regroup/Retry paths currently exist as native rear actions but lack equivalent menu dispatch; reuse their validated session methods. Menu opening is local; authority and command admission remain server-owned.

Accepted means admitted, executing means ongoing work, completed requires observed physical completion, and blocked explains a concrete condition. Missing gap data needs an explanation, not a misleading zero or an implication that travel is healthy. Keep leader calls restrained and consistent with actual downstream states. Preserve approved voice assets; no new recording requirement is needed for this iteration.

Check opening, selection, unavailable reasons, cancel/close, and restored walking/throttle/steering from on foot and a settled seat in the established lead. Respect inventory, map and GM input contexts. Menu access on foot does not automatically relax existing seated Resume or ownership rules.

## Complete journey and sequential bay workflow

Author a playable journey whose bay, waiting line, parking area and return departure are distinct, visible places. Establish a delivery-bay anchor before the player clears it; queued trucks must stay held rather than chase the lead as it moves away. Choose the simplest discoverable way to confirm that bay during implementation, based on the spoken workflow and installed engine behavior.

1. **Prepare and recruit.** Place ordinary drivers and loadable supply trucks with real source reach. Use native loading and existing recruitment/boarding. Confirm the intended assigned vehicle and original driver seat before travel. Preserve the useful quick interaction; acquisition redesign is deferred.
2. **Drive outward.** The player leads. Followers start promptly, maintain the predecessor chain, negotiate turns/grades and respond to suitable pace changes. Preserve useful road following when the player takes a harmless shortcut; an untraversable deviation should produce an understandable state. Commands and pacing advice must help manage travel instead of masking sustained poor driving by requiring constant waiting.
3. **Player enters the bay.** The player manually drives their truck into the unloading position. AI trucks remain seated and held in the queue. The interface identifies the occupied bay and the next waiting unit. Do not take control of the player lead.
4. **Player unloads and clears.** Use the native rear supply action, verify actual destination transfer and stable quantities after cancellation, then manually drive the player truck into clear parking. Physically verify that the bay and approach corridor are free.
5. **Advance exactly the next AI truck.** An explicit owner command admits the original next queued truck into the vacated bay; other trucks keep their seated Hold. No automatic advance during player unloading or simply because another truck reports a release event. Confirm approach, stationary arrival and native destination reach before unloading.
6. **Unload and park that AI truck.** The player uses native unloading while the driver remains seated and held. Command it to park clear only after a safe plan is admitted. Observe the original truck leaving the bay, reaching actual clear space and staying parked; an accepted order or distance from an anchor is insufficient. Keep the remaining line held if the maneuver fails. Repeat steps 5-6 for the other AI trucks.
7. **Regroup and return.** With the bay sequence complete, the player settles in the original lead and deliberately regroups the parked convoy. Retain driver/truck identities and assignments while reconnecting the physical predecessor order. The player turns/departs and leads the same convoy back to original staging. Require sustained homeward travel and a final stopped chain, not only a merge event or a short Resume movement.

For the first complete demonstration, use a supply-capable player lead and one AI truck, each natively loaded to 100. Proposed source/player/AI/destination ledger: **1800/0/0/0 -> 1600/100/100/0 -> 1600/0/100/100 -> 1600/0/0/200**; those final counts persist through parking and return. This deliberately tests the player's unloading example and the mandatory loaded AI journey; it is not a claim that the recorded example delivered those amounts. An empty-lead variant can follow later without changing queue semantics.

For player plus N AI trucks each loaded to 100, track every original container: final source **1800 - 100(N+1)**, all trucks **0**, destination **100(N+1)**. One/two/three follower cases exclude the player lead. Preserve partial deliveries and rejected/canceled transfer history. No runtime count setters, transfer fallback or truck repositioning may manufacture a pass.

Two existing handoffs need deliberate implementation:

- `CF_DriverControllerComponent.CF_CanReleaseAtUnload` rejects explicit Hold and currently instructs Resume first. Add a validated held-arrival -> commanded parking transition; retain Hold until a safe plan is admitted, then release only the owned selected truck's Hold/Wait. Do not globally weaken guards or dismiss/recruit to move it.
- `CF_ConvoySession.OnUnloadBayCleared` currently calls `ResumeUnloadQueueAtAnchor`, with other queue-resume call sites. Separate clearance eligibility from owner permission to advance the next truck. Preserve session/assignment invariants and audit those other paths so one cannot bypass the explicit queue command.

Existing rear pull-off means a slot behind the convoy and can require a tight turnaround. It is not already a verified intuitive shoulder-parking operation. Choose an actually usable pocket/route; measure truck bounds, obstacles, bay and road corridor clearance, stationary dwell and ability to depart. If suitable space is unavailable, retain a safe stopped state and explain it.

## Varied non-Arland integration route

Propose a **new Everon daylight journey**, using Regina and Levie as geographic anchors and existing terrain knowledge as a head start. Select the actual route, supply origins, waiting/bay/parking layout and turnaround during the implementation session after inspecting terrain/nav and physical vehicle access. A route through the Regina exit, the Green Valley/Peters Pass area and a suitable delivery area near Levie is a candidate, not a verified route commitment.

Make this a longer representative outward/delivery/return journey with meaningful turns, grades, different road widths and a genuinely drivable unpaved section. Use appropriate pace changes on open straights, turns, climbs and the bay approach, with actual speeds recorded. Avoid substituting several kilometres of repetitive straight road for variety. The narrow old Levie apron may need a different nearby bay/parking site.

Reuse the old Regina/Levie map knowledge, not its readiness claims. The existing night world does not prove field exit/nav, LAV seat spawn, daylight, supplies, turns or return clearance. Its loading distances and parent-container supply setup cannot be assumed to satisfy the later proven native child-container setup. A new daylight truck-first variant should establish real spawn/on-foot access, time of day, supply enablement, actual storage children, loading reach and clear staging. Do not merely rename the night scenario or make LAV seat spawning a dependency.

Vary plausible starting positions/alignment, route approach and lead pace across one-, two- and three-follower runs. Include an ordinary stop/restart and return traversal, retaining the predecessor chain. Record which starts and terrain were supported and which failed. Road-network distance or a grass-looking frame is not unpaved evidence: require moving wheel-material observations from participating trucks. No single route claims every edge case, speed or terrain.

Keep the short Arland loaded/native-lead fixtures as fast regressions and isolate a diagnosed startup, turn, stop or queue handoff in a small visible course when needed. They complement the primary varied journey; they must not consume the session in another unchanged geometry/input loop.

## Priorities, evidence and iteration decisions

1. **Understand and improve travel alongside command work.** Investigate the reported second-follower startup/blockage with matching identities/logs, then the slow following, large gaps and turn trouble. Compare role binding, predecessor history, native movement ownership, target lifecycle, speed/braking and geometry. Distinguish intentional waiting from failed progress. Preserve native steering/obstacle behavior where useful; deeper changes need physical comparisons. Do not diagnose solely from panel labels or solve travel by increasing speed/corridor thresholds.
2. **Make controls usable and destination phases coherent.** Implement the independent menu/state receiver and clear command scopes, plus the held-to-parking and explicit next-truck handoffs. Address physical driving defects uncovered by those phases as part of the same journey, not after a UI-only milestone. Preserve the server boundary and valued feedback.
3. **Complete the one-follower round trip, then the multi-truck queue.** Develop against the real non-Arland journey and ordinary player flow, using focused fixtures for specific failures. Extend to two and three followers with sequential actual delivery, parked clearance, regroup and sustained return. The known retained-tail parking/route ownership defect belongs in the multi-truck driving work; do not promote the unexercised private patch as a solution.
4. **Show and review the resulting trip.** Use the user's willingness to manually test/record for ordinary menus, walking, driving and unloading. Combine this with reproducible frozen-pack native-AI comparisons for movement/identity/cargo/hold. Disclose test-owner staging, scripted commands and any test assistance. Report what improved, the exact evidence and remaining limitations; do not promise general or multiplayer readiness.

Every experiment should state a gameplay question, an unchanged baseline and candidate, the observed result, and the next decision. If the relevant branch is not exercised, change the setup or hypothesis rather than accumulating more non-evidence. Do not add rigid artificial attempt limits, credit limits or a broad feature wishlist. Avoid repeated input calibration, isolated precision/proxy sweeps and diagnostic machinery that does not help this trip.

Retain historical acceptance gates and failures: 60 m per-link whole-run peak gap, 180 s arrival observation with <=2 m drift, >=30 s real Hold, and fresh >=20 m/three powered intervals where those contracts apply. Fresh movement is a restart qualification, not proof of return home. Preserve any stricter continuous activity/sequence requirements in the frozen comparison used. Longer representative runs must declare applicable criteria and report segment/whole-run failures without quietly weakening an old test or editing an old verdict.

Record actual cargo counts and transfer cancellation stability; powered signed progress toward the intended route; original owner/driver/group/truck/cargo identity, seats, assignments and predecessor links; gaps; stationary hold; actual bay/parking/corridor clearance; fresh return departure and completed staging arrival; meaningful player feedback and restored input. Retain the full console plus terminal snapshot and gameplay video. Distinguish fixture faults, convoy faults and automation limitations; classify gameplay, post-result and shutdown errors separately. Shared vanilla diagnostics can be attributed with evidence but must remain visible.

## Proven operations to reuse later

The saved project at `C:/Users/marco/Desktop/REFORGER` holds operational caches; the current managed worktree holds these documents and matching addon content. Choose one source checkout deliberately for later edits/builds; do not mix its resource database and pack with another build. Stop/close source Workbench/F5 safely before any source/resource edits. A standalone client may run concurrently with edits only when it loads a frozen copied pack in a separate addon directory.

```powershell
npm run cli -- doctor --mod 5A5FB20BD40C7C70
npm run workbench -- validate --project addons/ConvoyFollower/addon.gproj --execute
npm run workbench -- pack --project addons/ConvoyFollower/addon.gproj --output .cache/packed/<build> --execute
npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world <world-resource> --authorize-local-test-scripts --execute
npm run client:run -- --run-dir .cache/client/runs/<run> --world <world-resource> --addon 5A5FB20BD40C7C70 --addons-dir <frozen-addon-parent> --duration-seconds <safety-bound> --expect-game --execute
npm run logs:summary -- --log <console.log>
```

These are later-session recipes, not commands executed today. Pack already validates in a separate process; verify all five configurations and packaging. Deploy a unique `ConvoyFollower_5A5FB20BD40C7C70` child containing matching `addon.gproj`, `data.pak`, `resourceDatabase.rdb`, with source manifest and three hashes. Keep duplicate GUID copies out of that parent. Use isolated profiles and retain build/settings provenance. `client:run` supplies the required executable working directory; `--expect-game` verifies startup only. For an untimed manual handoff use `--keep-open` instead of `--duration-seconds`, state the session lifetime, and close normally after evidence.

The independent harness at `C:/Users/marco/Desktop/reforger-agent-harness` contains reusable launch/capture/snapshot lessons. Improve generic tooling there only when a concrete need justifies it; keep scenario-specific worlds/reporters here. `logs:summary` is an evidence index, and current `convoy:test` has no loaded-trip verdict. Reuse terminal snapshots/full logs rather than claiming a startup marker or legacy reporter passes the new journey. Refresh the observed target window before supported UI actions; do not repeat blind clicks or the failed scripted-owner control-write matrix. Use manual testing positively while automation delivery remains a separate limitation. Update the operations playbook when a new UI step is actually verified or fails in the later session.

## Starting prompt for tonight

> Implement and validate the September 29 Convoy Follower next iteration described in `docs/convoy-next-iteration-2026-09-29.md`. Read its copied playtest review, AGENTS.md and the current validation checkpoint first. This session has broad local implementation and machine-testing authority within the whole player-led supply journey: preserve the current addon, quick recruitment/boarding, predecessor chain/server ownership and leader voice while materially improving AI startup, pace, gaps, turns, stopping and restart together with commands and delivery flow. Do not stop at a menu-only milestone or reset to an old revision.
>
> Safely stop/close the source Workbench/F5 preview before addon/resource edits. Choose the source checkout explicitly, preserve current and private evidence, and use frozen packages with matching source/deployment hashes for comparisons. Reuse proven launch/harness lessons. Driver spawning/acquisition and WCS-specific integration remain deferred; do not contact others or publish.
>
> Implement a game-native, discoverable command interface usable on foot and seated where appropriate. A configurable standalone radial with contextual opener is the initial recommendation; an inventory item is optional. Preserve validated server commands, readable recipients, unavailable reasons, truthful accepted/executing/completed/blocked feedback and restored ordinary input.
>
> Complete real preparation/loading, dependable outward driving, destination arrival and sequential unloading, safe parking, regroup and player-led return to original staging with the same driver/truck identities and assignments. The player manually drives their own truck into the bay, unloads and drives clear; AI waits seated and the next original truck advances only on an explicit command after actual clearance. Keep Hold until a safe selected-truck parking plan is admitted. Use native supply transfers and conservation, without teleporting trucks, setting counts or taking autonomous control of the player lead. Complete one real loaded AI follower's round trip, then extend the same workflow and driving checks to two and three followers.
>
> Build a fresh daylight integration journey on another map, with Everon Regina/Levie knowledge as a starting proposal. Select actual road/nav/supply/bay/turnaround geometry through inspection and physical checks; the old night course is not a ready fixture. Include varied starts, meaningful road types, turns/grades, appropriate pace changes and a genuinely drivable unpaved section verified by moving wheel materials. Keep short controlled regressions to isolate concrete failures, but use the varied journey to expose practical driving defects early.
>
> Treat the recorded second-follower issue as undiagnosed until matching evidence explains it. Use manual user playtest/video for ordinary keys, clicks, driving and unloading, complemented by reproducible native-AI frozen-pack comparisons. Each experiment needs a gameplay question, baseline/candidate, result and next decision; preserve old gates/failures and do not repeat unexercised private branches or input-calibration loops. Record real supplies, powered progress, identities/seats/chain, gaps, hold, parked clearance, sustained homeward travel, useful feedback and separate gameplay/shutdown logs. Continue adaptively toward a materially improved complete trip, integrate demonstrated improvements and show the result with honest remaining limitations. Local success does not establish multiplayer readiness.
