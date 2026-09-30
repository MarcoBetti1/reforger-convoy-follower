# Player menu retest — September 30, 2026

Marco explicitly authorized a quick fix after his ordinary V8 playtest. The two long-running development goals remain paused. This is a narrow menu candidate and player handoff, not a resumed autonomous driving campaign.

## Human V8 result

Full log: `D:/ReforgerAgentRuns/everon-journey-manual-v8/logs/console.log`. Recruitment and boarding worked. Two settings-close records changed `CF_ConvoyCommands_click`, but no `COMMAND_MENU_OPEN` was logged. Marco reports tapping/holding F8 and rebinding without a menu. The old log has no callback/refusal-boundary instrumentation, so missing input delivery versus opening refusal is unresolved.

The driver entered FOLLOWING at15:46:23.661, ARRIVAL_APPROACH at15:47:53.478, ARRIVAL_RESUMED at15:48:00.561, and LOST at15:48:38.944. Marco reports slow following, unnecessary turns and repeated catch-up stops. These are a failed ordinary player journey; no delivery was completed. Final ledger source1500/lead0/follower300/destination0 conserves1800. Game destroyed at15:48:46.055. Retain full lifecycle errors: six SCRIPT(E) records during shutdown, plus other resource/default classifications; this is not a clean lifecycle.

Early calls were COHESION_ADVICE at gaps36.9272 and69.012m, later133.294m; they reuse range-warning audio. This differs from the180m sustained range-warning threshold and helps explain misleading feedback. The ordinary stop detector is six seconds and can replace a native follow lease with a stopped approach, then replace it again on resumed motion. Unnecessary-turn causality remains unproven. No driving, arrival or warning threshold was changed in this menu candidate.

## Candidate

`player-menu-overlay-v1` starts from the exact607-file frozen V8 source. Before preparation, the three edited files at HEAD were compared against V8 with line-ending normalization; no unrelated content differed. Changes are only the opening context priority5→50 with Overlay flag0x2, and registration/eligibility/request/refusal logging in the player controller and native radial. Overlay semantics are documented in the installed primary engine manual `D:/SteamLibrary/steamapps/common/Arma Reforger Tools/Workbench/docs/EnfusionScriptAPI/html/Page_Input.html`. Ordinary character/car contexts are priority10; radial context100. Owner, active-convoy, controlled-character and native-menu guards remain. Actual runtime priority suppression is a hypothesis until ordinary input is observed.

All five script configurations validated and packaging succeeded. Frozen607-source and230-packed-text checks passed. Exactly one GUID child in runtime parent `C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-menu-overlay-v1`; source archive outside that parent. Trio:

| File | SHA-256 |
|---|---|
| addon.gproj | 1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579 |
| data.pak | 04C792A6B5126294A589CC705BB1CC696FE039CD39E505A069BAD89FF8450814 |
| resourceDatabase.rdb | CFE16D8660DB5BE7A67633AE145DDD72C22E3D225E38DEBE6B586062C4AD4845 |

Preparation/provenance under `.cache/player-menu-overlay-v1` and `.cache/player-addons/player-menu-overlay-v1`. Frozen source `.cache/frozen-source/player-menu-overlay-v1/source` remains immutable during the client session.

## Open player session

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/player-menu-overlay-v1 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/player-menu-overlay-v1 --keep-open --expect-game --execute
```

Launcher session67613, no timer. GAME at16:00:08.555, COMMAND_INPUT_REGISTERED at16:00:08.559. Native UI enabled US Army via scenario properties, saved, joined Atlas Red2 and deployed at the exact named Regina staging spawn. Fresh capture shows first-person on foot with no editor overlay. Automated interaction stopped there; no recruitment, keypress, menu opening, command, cancellation or restored movement is claimed. Do not inject input or close the session without Marco's direction.

First retest: recruit the driver, press the assigned menu key once on foot, close the radial and check ordinary movement, then repeat while seated. If it fails, inspect COMMAND_INPUT_ELIGIBILITY, COMMAND_MENU_REQUEST and COMMAND_MENU_REFUSED to distinguish missing routing from a guard/display failure. A nearby contextual “Convoy commands” action is an existing alternate route, not yet human-qualified. Driving remains failed and must not be relabeled fixed by this menu change.
