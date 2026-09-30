# Convoy control point — September 30, 2026

The latest human test is closed; the mod is a development build. Ordinary menu input and the read-only debug HUD are human-confirmed. Hold/Resume and lost recovery improved, but the bay still fails native completion/readiness and driving still shows braking/gap lag. Alternate voice selection failed because the test override was in the wrong profile folder; its corrected location takes effect at the next startup and needs playback confirmation. No release-ready claim.

## Next focused work, already requested

1. Finish the unload queue: lead parks; owner can leave the seat, unload an admitted truck and command it from the rear or wheel to the return waiting line; then admit the next truck without canceling the whole bay. Diagnose the native stopping reference before changing geometry; retain honest blocked reasons and original identities/cargo.
2. Make voice selection/playback observable, retain the original recording and add contextual alternatives. Prevent cohesion escalation and true-range audio from repeating the same warning within seconds. Existing generated pack has66 clips; additional events need real state hooks.
3. Replace vehicle-crew/pilot-like clothing with practical faction-appropriate driver uniforms, preserving seat/AI controls and convoy actions.

Further pacing experiments are deprioritized. Existing driving limitations stay documented; they are not proven unavoidable engine limits. The broad autonomous goals remain paused. The focused requests above are authorized, with user testing at the next prepared-build handoff.

## Working locations

| Purpose | Location |
|---|---|
| Current addon implementation | `C:/Users/marco/.codex/worktrees/175a/REFORGER`, branch `codex/convoy-player-trip-20260929` |
| Control notes and reusable local launch tools | `C:/Users/marco/Desktop/REFORGER`, branch `codex/release-hardening` |
| Independent generic harness | `C:/Users/marco/Desktop/reforger-agent-harness`; prior dirty docs preserved, handoff branch pushed |
| Current frozen player package | Implementation checkout `.cache/player-addons/player-debug-bay-v1` |
| Chosen rollback package | Implementation checkout `.cache/player-addons/player-stop-continuity-v1` |
| Completed latest session and full log | `D:/ReforgerAgentRuns/player-debug-bay-v1/logs/console.log` |
| Correct voice override | `D:/ReforgerAgentRuns/everon-journey-manual-v8/profile/profile/ConvoyFollowerSettings.json` |
| GitHub-verified exact package/log archives | `D:/ConvoyTakeoverBackups/2026-09-30` and the repository's draft takeover release |

No game or Workbench source session is open. Keep one uniquely named candidate and one frozen runtime for each new user handoff; use an untimed client and let Marco control deployment and testing. Do not edit frozen runtime files.

Start with [current validation](convoy-validation-current.md), then [the exact player checkpoint](convoy-player-debug-bay-2026-09-30.md) in the implementation checkout. Earlier dated pages are historical, not current launch directions. The [storage audit](cleanup/convoy-storage-audit-2026-09-30.md) records71.88GB removed and provides the complete deletion/restoration index. Historical raw videos were deliberately retired; retained summaries/images are not a substitute for deleted full footage.
