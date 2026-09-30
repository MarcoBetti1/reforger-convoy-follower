# Player debug and unload-bay retest — September 30, 2026

Marco closed the earlier human session and explicitly authorized this patch and an untimed replacement. Long development goals stay paused. Source checkout: `C:/Users/marco/.codex/worktrees/175a/REFORGER`; branch `codex/convoy-player-trip-20260929`.

## Earlier session closed result

`player-stop-continuity-v1` reached GAME16:46:58.591 and closed normally17:07:55.007, launcher exit0. Full console retained at `D:/ReforgerAgentRuns/player-stop-continuity-v1/logs/console.log`. Errors before the WORLD_CLEANUP boundary: 31 total / 0 SCRIPT. Cleanup/shutdown: 70 total / 2 SCRIPT. Normal closure is not an error-free lifecycle.

Human: Admit next worked; the admitted truck stopped outside the recorded bay readiness area. Cancellation allowed continued driving. Repeated stop/restart braking remains on hills and, less noticeably, downhill. At steady modest speed the follower maintained an initial gap; the driver's estimated10–15mph is not calibrated HUD evidence. Multiple cohesion calls repeated roughly30seconds apart. These are failures/observations, not a driving pass.

The bay saved7163.32/140.902/2235.68; road projection7161.26/140.829/2237.22. The truck settled7154.26/141.36/2236.13: about7.1m from the projection but9.1m from the bay. Controller braking used the first8m center while session readiness used the second. A replacement MOVE restored20m instead of the original4m. No exact heading predicate caused this rejection.

## Authorized changes

- Explicit bay braking and arrival check the intersection of the existing8m saved-bay and road-goal areas. Completed-arrival hysteresis retains12m, now checked against both centers. No readiness radius is widened. Every newly created bay MOVE receives4m before it enters the AI group, and lead movement cannot reset it to20m.
- Cohesion defaults: ease60m, wait90m, clear40m, prediction1.5seconds, radio cooldown60seconds. Audio requires actual gap at least60m, allows first/escalation call per pressure episode, and rearms only after5 continuously measured seconds below clear. Stops or interrupted samples cannot rearm it. True180m/3second range warning and275m/10second loss gates remain; real far warning is no longer muted by visual advice alone.
- Read-only owner HUD defaults visible in this candidate. Maneuver options → Hide/Show driving debug toggles it locally without submitting a driving command. Updates at2Hz; stale data labeled after3seconds. North-up plot shows first active truck (yellow), its current target or bay road goal (green), actual native movement path (cyan, bounded24points), gap, actual km/h, cruise request, brake/throttle/steering/gear/handbrake, spacing advice. Additional active units get compact status lines. Cruise request is not measured speed or proof the AI will accelerate. Native path unavailable/empty is shown as zero points, never fabricated.
- This test's existing isolated profile now overrides only `m_iVoicePack:1`, enabling the already included generated alternate leader clips. Original recordings/default remain available. Profile path: `D:/ReforgerAgentRuns/everon-journey-manual-v8/profile/ConvoyFollowerSettings.json`.

Native braking and gap closing are **not fixed or validated** by this change. Native steering, throttle, braking, vehicle ownership and final stop capture remain in control. Diagnostics use documented read-only [native path](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceAICarMovementComponent.html) and [vehicle simulation](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceVehicleWheeledSimulation.html) APIs.

## Reproducible build

Candidate `player-debug-bay-v1`; all WORKBENCH/PC/XBOX/PS4/PS5 validation and pack succeeded in `.cache/workbench-runs/player-debug-bay-v1-repair1`. Initial compiler rejection from a float/string path serialization was repaired; initial failed log remains separately in `.cache/workbench-runs/player-debug-bay-v1`. Frozen source607files and230packed texts verified. Runtime parent `.cache/player-addons/player-debug-bay-v1`, one `ConvoyFollower_5A5FB20BD40C7C70` child.

- `addon.gproj`: `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579` (135 bytes).
- `data.pak`: `F0E5F10FA5AAA4479D427AFDAD6BB73DF2661F56EFA1D47990EEF5F3339C2E49` (32826735 bytes).
- `resourceDatabase.rdb`: `CFE16D8660DB5BE7A67633AE145DDD72C22E3D225E38DEBE6B586062C4AD4845` (34591 bytes).

Launch after remote backup:

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/player-debug-bay-v1 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-debug-bay-v1 --keep-open --expect-game --execute
```

Marco owns deployment, placement and input. Startup and a rendered HUD are not bay completion, smooth driving or a multi-truck pass. Retest bay admission and release/parking, then steady driving and one uphill stop/restart while noting actual brake and cruise request. Listen for frequency and wording of calls. Do not advance to video/multi-truck acceptance on compilation alone.

## Backup coverage

Code/checkpoint will be pushed before handoff. New archive includes exact frozen source/runtime/provenance, build logs, previous complete human console and profile override; full entry decompression and GitHub digest will be verified. Earlier archives stay retained.221raw videos (~68.4GB) remain local, outside the GitHub backup. Preparation is complete; startup/human result to be appended after launch.

## Remote backup and open retest

Code/checkpoint pushed as241bff9/d528362. The draft takeover release contains `convoy-debug-bay-v1.zip`:618 full-readback-verified payloads,56922653bytes, SHA-256 `0AB99C0EABE31458B58E47A1109FAEBEC61010911361CAF259301A71232CD984`; uploaded size/digest matched. It includes the complete previous human console (normal closure/retained errors), final successful build logs, exact source/runtime/provenance and profile override. Earlier artifacts remain.

Untimed client launched17:24:01 and reached GAME17:24:15.636; command listener17:24:15.639. Launcher session45042, client PID36616. Full live log `D:/ReforgerAgentRuns/player-debug-bay-v1/logs/console.log`. Exact frozen project/package/RDB mount confirmed and all three hashes rechecked unchanged after startup. Read-only native window capture showed rendered Regina Game Master scenario-properties UI; no automated deployment or input. Owner HUD requires controlled character, so no rendered HUD/path/control result is claimed yet. Profile override is selected on disk; runtime SETTINGS_PROFILE_APPLIED/voice playback awaits convoy settings initialization. Human bay/driving/warnings/voices/debug validation remains pending. Marco retains full control.

## Open human debug/bay feedback

Marco confirms the debug display works and says following may feel better, but this is not a spacing/drive PASS. He reports the admitted truck visibly pulled up to his bay, then could not find the normal completion path and Resume showed finish-waiting. He continues voice testing and observes brake input with a catching-up gap. No game input, source edits, package replacement or new launch occurred during this observation.

The live log shows the rejected command was5 (Resume ahead line), which is separate from the explicit bay's rear-parking procedure. Intended path: native unload → Units / Unit One / Pull off-regroup → wait for rear parking → Regroup for return. However **explicit readiness still fails**, so that path may remain unavailable: no EXPLICIT_BAY_SETTLED/selected bay Wait before cancellation. Rebuilt bay MOVE now logs radius4 as intended, but native completion leaves original truck7164.07/140.441/2242.46, road goal7160.27/140.961/2235.92 (7.59m center distance), saved bay7163.18/141.013/2233.56 (about8.96m). Native completion geometry still stops outside the saved8m gate. Do not attribute completion to user misunderstanding or widen readiness based on a visual spot match. The exact native stopping reference/vehicle-footprint contribution remains unisolated.

Cancel bay command6 accepted17:30:02.706, retained original assignments/explicit Hold, then actual Resume all command1 accepted17:30:20.240. Measured follower speed rose from0 to27.79km/h while lead was stopped. Subsequent sampled braking17:30:31.289 at gap27.59m, actual17.20km/h, cruise request21.72km/h shows brake0.736 and throttle0. That observation does not identify which native driving reaction requested braking or establish an engine limit. These are open-session samples, not a complete physical trip result. Partial console snapshot is labeled `console-PARTIAL.log` under `.cache/player-debug-bay-v1/human-open-observation`; full live console remains authoritative.

No SETTINGS_PROFILE_APPLIED line observed in the sampled startup/convoy log; alternate playback is still unverified despite the disk override. Check local-session settings initialization/profile mapping before claiming generated clips played. Human voice/warning feedback pending.

## Closed human result and revised priorities

Marco heard only his original range-warning recording, including two closely spaced calls. Live log records cohesion escalation17:39:00.220 then true range warning17:39:07.437; these separate event IDs can still produce adjacent same-wording audio. Profile setup error isolated: engine mounts `$profile` at `D:/ReforgerAgentRuns/everon-journey-manual-v8/profile/profile`, while the override had been one level above. The JSON has now been moved into the mounted subfolder with matching hash; runtime loads it at next startup. No new alternate playback is claimed. This corrects earlier preparation's wrong path. Original assets remain, and existing generated pack has66 clips/two variants per33 call positions; additional contextual lines should be wired only to verified state transitions.

Marco wants the player to park the lead, get out and unload each admitted truck, then command that truck from its rear or wheel into the rear waiting line, preserving the queue for the next truck. Cancellation must remain recovery, not the normal multi-truck completion flow. Drivers inherit US/USSR Crew base prefabs; current US test driver likewise inherits Crew. Clothing needs a targeted loadout change without removing driver control/seat/actions. No prefab or driving source changed during this audit.

User closed the session normally17:42:22.790; launcher exit0. Full `D:/ReforgerAgentRuns/player-debug-bay-v1/logs/console.log` retained. Errors before WORLD_CLEANUP: 21 total / 0 SCRIPT. Cleanup/shutdown: 69 total / 2 SCRIPT. HUD human-confirmed; native bay readiness and brake/gap issues remain failed. Voice alternate selection failed this run due the wrong override location. Follow-up priorities above supersede further pacing experiments.

Marco separately authorized storage cleanup; [the audit](cleanup/convoy-storage-audit-2026-09-30.md) and deletion index describe retired recordings and deduplicated packages. This exact current frozen pack, rollback pack, source and full logs were preserved. Historical 'all raw videos retained' backup descriptions no longer describe the post-cleanup filesystem.
