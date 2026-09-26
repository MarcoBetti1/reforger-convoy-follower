# Native lead hold investigation

Updated September 26, 2026. This is a test fixture experiment, not a production follower change or a gameplay pass.

## Installed implementation evidence

The installed game sources were read selectively from `D:/SteamLibrary/steamapps/common/Arma Reforger/addons/data/data007.pak`; only the named source records were decompressed in memory. Behavior trees came from `data.pak`. No base-game source was added to this repository.

- `scripts/Game/AI/Components/SCR_AIUtilityComponent.c`: `GetCurrentBehavior()` returns the selected behavior. `CancelAllGroupActivityBehaviors(groupUtility)` fails registered behaviors whose related group activity belongs to that utility. `EvaluateBehavior()` ticks its private callback queue, processes one mailbox message, removes obsolete actions, and selects a behavior. Cancellation of currently registered actions cannot remove a goal message that has not yet been consumed.
- `SCR_AIGroupUtilityComponent.c` and `SCR_AIWaypointState.c`: removing/deselecting a current waypoint invokes `CancelActivitiesRelatedToWaypoint(..., doNotCompleteWaypoint: true)`. The flag breaks the activity's waypoint reference before failure, avoiding a second completion of the removed waypoint.
- `SCR_AIMoveActivity.c`, `SCR_AIActivity.c`, and `SCR_AIGoalReaction.c`: move completion/failure sends cancellation messages to agents; the goal reaction later fails behaviors related to that activity. This is an asynchronous path.
- `SCR_AIUtilityComponent.c` creates `SCR_AIIdleBehavior_Driver` when a pilot enters a vehicle, with no related group activity. Group-activity cancellation leaves that idle behavior available. It enables combat movement and uses `VehicleInCombat_DriverListener.bt`.
- `SCR_AIBehavior.c` provides `SCR_AIWaitBehavior`. Its native `AI/BehaviorTrees/Chimera/Soldier/Wait.bt` continuously returns RUNNING. This gives the fixture an explicit selected behavior to own waiting while the prior movement branch is deselected. Whether that transition actually arrests and holds native vehicle movement remains a physical test question.
- `SCR_AIGetInVehicle.OnActionFailed()` can eject an already seated character. The fixture therefore calls public `RemoveObsoleteActions()` before cancelling current group-related driver behaviors, avoiding re-failing completed boarding actions.

## Hypothesis and concrete test

The late reverse may come from movement that survives the queued waypoint cancellation or regains ownership after the old waypoint is gone. A cruise-zero request and repeated simulation brakes do not identify or remove that owner.

The revised fixture verifies that its one-member pilot group owns the seated pilot and that the native car mover owns the named lead truck. It logs current/group behaviors and all pilot actions, cancels the waypoint activity without completing it, removes that waypoint, directly cancels current group-related driver behaviors, then adds one test-owned native wait at priority level 2100. A single cruise-zero and handbrake request supplement this explicit behavior owner. There are no continuing POSTFRAME brake writes after the driving phase begins.

Physical acceptance is at least 30 seconds after a settled pose is captured, with at most 2 m horizontal displacement from that pose and the native wait still selected. The native wait is failed explicitly for restart, the owned cruise override is reset, the handbrake is released, and a new native MOVE is issued. A surveyed 40 m extension preserves the original first leg. An unavailable extension fails the fixture before any convoy order.

Restart requires at least 20 m signed lead progress, three forward samples, and two powered samples (engine on, forward gear, throttle, speed, positive progress). The lead completion marker is independent of the follower's separate 15 m signed progress gate. The combined scenario also requires a second 30-second hold and the existing distance/path/settling checks. After its provisional terminal PASS it continues physical hold logging for 30 more seconds; a later drift remains a fixture failure.

Markers retain `OFFROAD_*` and add `hold_restart=true`, hold `cycle=1|2`, absolute probe `seconds`, before/after behavior logs, waypoint lifecycle events, independent lead restart completion, and explicit fixture failure/observation completion. The strict 60 m peak-gap and runtime/lifecycle error gates remain the reporter's responsibility. The historical Arland world crosses concrete taxiway and must still be reported as off-network, not proven unpaved.

## Status

The root agent's v2 frozen pack passed all five compile configurations and packaging. Its `data.pak` SHA-256 is `D17AEB665902501A629D735466FA67E3982891D532FCB24E389D1D65C8C6CA50`. The root agent launched and observed `.cache/client/runs/native-lead-hold-v2/`; this investigation did not launch a game or Workbench itself.

The live log establishes the narrower lead fixture result:

- First hold began at probe second 33 and completed at 63: 30 seconds, 0 m drift, native wait selected.
- Native lead restart completed at second 69: 24.502 m signed progress and five powered samples. Combined restart completed at 73: lead 39.9711 m and follower 17.8267 m.
- Second hold began at 74 and completed at 104: 30 seconds, 0 m drift. The further 30-second observation completed at 134 with `failed=false`; the second hold remained at 0 m drift.
- Final lead/follower paths were 114.510/119.921 m, goal gap 5.804 m, link gap 14.401 m. Peak separation was **75.712 m**, exceeding the unchanged 60 m gate. The provisional scenario PASS does not make this an overall product pass. Runtime and shutdown classification belong to the full report.

The behavior logs also narrow the investigation. At the first stop, the waypoint was already removed and the current behavior was unrelated `SCR_AIIdleBehavior_Driver`; group cancellation did not alter it. Before restart, a new unrelated `SCR_AIPilotMoveFromIncomingVehicleBehavior` appeared in the action list while the owned wait remained selected. At the second stop, a still-running `SCR_AIMoveInFormationBehavior` remained after waypoint removal, and direct cancellation changed its state from RUNNING to FAILED. These are distinct sources of movement ownership.

Selective inspection with the new independent harness reader confirms the pilot avoidance behavior in `scripts/Game/AI/Behavior/SCR_AIMoveFromDanger.c`. Unlike the on-foot variant, it has no `CustomEvaluate()` expiry condition. When selected, it creates an avoidance target from the current vehicle pose, often 8 m right and 12 m ahead. `scripts/Game/AI/Reaction/Danger/SCR_AIDangerReaction_Vehicle.c` creates it with a null group activity after a predicted collision within 10 seconds. A delayed autonomous avoidance request is therefore a concrete hypothesis for the old late reverse. The v2 logs demonstrate its existence, not its selection in the old failed run.

After freezing v2, two small source changes were prepared for the next pack: refuse cruise reset if the recorded native mover no longer controls the lead, and require at least 0.25 m signed movement for each powered restart sample to match the reporter. These changes are not present in the v2 hash above. Production follower behavior remains unchanged by this investigation.
