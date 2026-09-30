# Human short trip and next candidate — September 30, 2026

Marco reported a successful short trip on `player-command-start-v1`, including Hold/Resume halfway, recovery after initially getting lost, and cancellation of an unsuccessful bay followed by resumed driving. This is positive human experience on one trip, not autonomous repeatability or a full supply journey. No stale-failure poisoning was reported on this trip; it does not prove recreation of the previous dismissal/recruitment failure.

## Bay failure and menu changes

The live console records Start accepted16:27:59.092, Hold16:30:06.675, real seated Hold16:30:08.824 and Resume16:30:17.241. Resume completed measured powered progress7.40661m at16:30:21.041. Set bay recorded the actual lead pose at16:30:31.108 and held Unit1 at16:30:32.025. Bay admission attempts16:31:36.858,16:31:58.392,16:32:21.024 and16:32:36.358 failed the engine `GetReachableWaypointInRoad` query. No “mark moved out” command is required: recording saves the position, clearance is automatic, and Admit requires the lead stopped and seated clear of the approach. The route-query failure does not prove terrain type or which endpoint lacked a road connection.

Cancel bay at16:32:39.507 retained original assignments held. Resume16:32:43.924 completed measured powered progress5.87844m at16:32:48.791. Supply ledger later reads300source/0lead/1500follower/0destination, total1800. Do not claim destination delivery from the owner's report of unloading his lead.

Authorized changes now place individual categories under **Units → Unit number → command**, keeping one Units sector on the main wheel as the roster grows. Existing identity/status updates remain in place. Native icons now use the exact GUID/name pairs from installed base-game commanding configs; filename-only references caused478 resource errors in the human session. Symbols remain visually unverified in the replacement.

A read-only helper exposes the existing8m reachable-road query and5m saved-bay projection tolerance. Set bay now rejects a failed front-truck road preflight before changing the queue or recording a bay. Admit exposes the same specific reason. Final goal/lead clearance, vehicle-bound corridor checks, original driver ownership and native movement checks remain unchanged. This is early validation and better feedback, not a new off-road navigation system. Instructions state that bay storage and clearance are automatic, and Resume requires ending the bay maneuver.

## Stop/restart investigation and focused candidate

Marco also reported trailing behavior uphill and a pause when restarting after letting the follower catch up. The native activity follows the current predecessor entity; it is not replaying a fixed20-second input recording. Logs show unwanted braking during order turnover: at16:35:00.190 and16:35:36.557 arrival resumes, the old waypoint is cleared and a fresh moving-follow lease binds; nearby speed samples fall sharply despite a high requested speed cap. That correlation identifies a candidate mechanism, not the entire hill-driving cause. The leader/follower physics and gap still require a live comparison.

`player-stop-continuity-v1` includes the Units/bay/icon changes plus one narrow arrival adjustment: while an exact healthy executing moving-gap lease is still approaching a stopped predecessor, defer replacing it with the tighter stopped-gap order until the truck is near and slow. The existing cruise envelope still brakes for the actual predecessor. If the predecessor moves again, existing exact lease-retention logic can preserve movement rather than recreate it. Failure, Hold, captured Wait, ownership and final slow/stable arrival checks remain. Moving/stopped gaps and warning thresholds are unchanged. No predictive shortcut or terrain/path bypass was added.

The intermediate `player-units-bay-v1` and final `player-stop-continuity-v1` passed all five script configurations and packaging. Final607 source/230 packed-text verification and trio hashes passed. Final runtime parent `.cache/player-addons/player-stop-continuity-v1` contains one GUID child:

| File | SHA-256 |
| --- | --- |
| addon.gproj | `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579` |
| data.pak | `2C87FD7FE5C963E4DB7AE8C5B862F28BECB08B8B5B0B396928AC96662BAC736A` |
| resourceDatabase.rdb | `CFE16D8660DB5BE7A67633AE145DDD72C22E3D225E38DEBE6B586062C4AD4845` |

Marco subsequently requested the launch. All three frozen deployment hashes were rechecked, no existing client was found, and the untimed replacement launched with the existing isolated human profile and ordinary Regina supply world. Console confirms the exact frozen package/RDB, GAME16:46:58.591 and input-listener registration16:46:58.594. Launcher session39788; run/log root `D:/ReforgerAgentRuns/player-stop-continuity-v1`. No deployment or player input was injected; Marco owns placement/testing. **Startup is verified; replacement gameplay remains unverified.** Broader development goals remain paused.

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/player-stop-continuity-v1 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-stop-continuity-v1 --keep-open --expect-game --execute
```

## Closed human evidence and next test

The human session closed normally: WORLD_CLEANUP16:38:20.326, `Game destroyed`16:38:21.302, launcher session33018 completed with exit0. Full console `D:/ReforgerAgentRuns/player-command-start-v1/logs/console.log` retains500 gameplay error records (zero gameplay script errors) and74 shutdown error records (six shutdown script errors). The478 bad icon references are part of those gameplay errors. Normal closure does not mean an error-free lifecycle.

Retest the same hill with one follower: stop briefly to inspect it, then restart while it is still approaching; compare actual braking, separation and retained activity generation. Also stop long enough to confirm final arrival still settles. Test the Units submenu with2–3 units. For bay testing, choose an engine-reachable road spot, set the bay, unload the lead, drive clear and stop seated, then Admit; unload the follower and command parking before return. Exact route rejection should now appear in the wheel if the spot cannot be reached. Full trip, parking/return and uphill continuity remain open until physically observed.

The frozen replacement and full closed human log are saved to the existing GitHub draft takeover release as `convoy-bay-continuity-v1.zip`. All617 payloads passed full decompression/hash readback against originals; uploaded size56873006 and SHA-256 `03520AA0646D49EE43FD7FD7DBB92AAE64D27B9FD7F5AF1F439F1316F978F146` match the GitHub asset digest. `bay-continuity-asset.json` records verification. Code was pushed as `7c1d9b4`; the control handoff as `9a8bcdf`. This later documentation records completed upload verification; the archive contains the preceding checkpoint version. Raw video coverage remains as previously documented.
