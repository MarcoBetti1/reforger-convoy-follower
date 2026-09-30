# Player Start and recovery candidate — September 30, 2026

Marco confirmed ordinary key input opens the menu in `player-menu-overlay-v1`. During that human session a follower stopped after a movement failure and rejected Resume even after dismissal/recruitment. Marco did not intentionally issue Hold; an accepted Hold record does not establish player intent. He authorized a focused recovery repair, explicit Start and easy native symbols, keeping the current layout and hover explanations. Long development goals remain paused. This patch does not establish a driving or release PASS.

## Root cause and scope

The live overlay console records the generation2 movement rejection at16:04:02, dismissal at16:07:20, Idle reset at16:07:23.950 and a new boarded assignment at16:07:30.734. Resume at16:10:28,16:10:34 and16:14:35 rejected `unattributed_entity_fallback_failure`. Idle reset cleared the original-follow block but left the active entity-fallback failure flag set. The new reset clears that active flag only after the old lease/slot safely retires; the historical failure remains recorded. A surviving active failure now explains that the driver needs dismissal and recruitment. This repairs stale state across assignments, not the underlying native path failure or physical obstacle.

Fresh vehicle assignments now wait for **Start convoy**. Start authorizes the existing safe predecessor-departure checks; it does not issue movement immediately or relax distance/ownership checks. Recruit and board the driver, enter the original lead truck, choose Start convoy, then pull forward. Use Resume after holding an already started convoy. Foot following is unchanged. Native arrows, halt, follow, get-in, patrol and supply symbols replace the identical dots where available. Hover explanations and layout remain. Arrival, warnings, route and driving thresholds were not changed. Older autonomous fixtures that assume automatic departure need explicit Start before future reruns; their prior frozen evidence remains intact.

## Build and exact running package

Candidate `player-command-start-v1` passed WORKBENCH, PC, XBOX, PS4 and PS5 script configurations and packaging. Both validation and pack retained25 resource error records and zero script errors, matching the menu overlay baseline; this is not a clean full lifecycle. Build logs are under `.cache/workbench-runs/player-command-start-v1`.

Frozen607-source-file and230-packed-text comparisons passed. Runtime parent `.cache/player-addons/player-command-start-v1` has one GUID child `ConvoyFollower_5A5FB20BD40C7C70`. The deployment trio was rehashed after launch and matched `verified-provenance.json`:

| File | SHA-256 |
| --- | --- |
| addon.gproj | `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579` |
| data.pak | `B1C0D42F691BA4D185D17B07AF688B6F084F36574F41259F70A488924EA64ABC` |
| resourceDatabase.rdb | `CFE16D8660DB5BE7A67633AE145DDD72C22E3D225E38DEBE6B586062C4AD4845` |

Frozen source: `.cache/frozen-source/player-command-start-v1/source`. Source manifest and pack verification are saved beside the runtime parent.

Launch from `C:/Users/marco/.codex/worktrees/175a/REFORGER`:

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/player-command-start-v1 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-command-start-v1 --keep-open --expect-game --execute
```

Launcher session33018 is untimed. Console `D:/ReforgerAgentRuns/player-command-start-v1/logs/console.log` confirms the exact addon, package mount and resource database; GAME at16:21:44.565 and command listener registration at16:21:44.568. Initial Game Master input lag prevented an observed deployment; Marco restored responsiveness himself and requested control of placement/testing. Logs subsequently advanced. **Stop automated input here: do not deploy, recruit, place, close or replace the game unless asked.** Startup does not establish Start, Resume recovery, symbols or driving behavior.

## Previous session closure and evidence

The complete closed overlay console remains `D:/ReforgerAgentRuns/player-menu-overlay-v1/logs/console.log`. It ends at16:19:40.080 following world cleanup at16:19:39.286, with no `Game destroyed` marker. Launcher exit3489660927 is nonzero. The full console retains2 script error records and22 total error records, including shutdown resupply-catalog errors. Preserve these separately from the player's menu-opening success and recovery failure.

GitHub backup archive `convoy-start-recovery-v1.zip` contains this exact frozen source and runtime trio, provenance, build logs, the complete closed overlay log and an explicitly partial snapshot of the still-open replacement log. Verify ZIP entries against originals and the uploaded asset digest before calling the backup complete. Existing raw videos remain local as recorded in the takeover checkpoint.

## Human checks still pending

1. Recruit and board a fresh driver: verify the truck waits before Start. Enter the original lead, choose Start and drive forward; check actual movement and menu symbols.
2. Hold an already started truck, then Resume from the lead; verify it follows physically.
3. If a movement failure occurs again, safely dismiss and recruit the driver, choose Start and test actual recovery. Capture the exact rejection if it persists. The patch is not proof that an actively stuck vehicle can recover.

General following, arrival timing, warning distance and complete supply journeys still need real driving feedback. Keep the full replacement console after player exit and separate gameplay from shutdown errors.
