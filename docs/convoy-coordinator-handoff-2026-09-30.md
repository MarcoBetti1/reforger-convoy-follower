# Convoy coordinator pause handoff — September 30, 2026

Marco requested wrap-up, GitHub synchronization and paused goals through **Arma / Reforger Control**, chat `01a0f3e6-7755-7ae3-9e16-8d88e57a6a4b`. That chat owns subsequent machine coordination. Resume development only after Marco explicitly asks. No further game launch is part of this checkpoint.

## Repository ownership and resume context

- Main checkout: `C:/Users/marco/Desktop/REFORGER`, branch `codex/release-hardening`, GitHub `MarcoBetti1/reforger-convoy-follower`. Its pre-checkpoint parent is `27e30687ec213c4058031c23db390d0a495b1c50`. This checkpoint archives the previously outstanding retained-arrival OFF_ROUTE JSON/patch and its byte-preserving attributes; it does not apply or promote that experiment.
- Current development checkout: `C:/Users/marco/.codex/worktrees/175a/REFORGER`, branch `codex/convoy-player-trip-20260929`. **Redesign convoy controls and complete…**, chat `01a0eeb9-21cd-7f63-9b98-de2e1bd11900`, owns that checkout and the generic harness changes. It is saving its own source, evidence summaries, commit/push proof and paused-goal handoff.
- Generic harness: `C:/Users/marco/Desktop/reforger-agent-harness`, GitHub `MarcoBetti1/reforger-agent-harness`; its sync belongs to the development chat.
- The development checkout's `docs/convoy-validation-current.md`, `docs/convoy-feature-courses.md` and `docs/experiments/player-trip-20260929-session.md` contain the latest development evidence. Main-checkout release-hardening records describe the preserved earlier baseline, not the newer player-trip build.
- The overall control chat will record the final verified remote commit IDs after both owners finish. A local main-checkout push proof is saved at `.cache/coordinator-pause-20260930/push-proof.json` after a successful normal push. Generated packs, raw logs and recordings remain at their preserved local paths; GitHub source/summary synchronization does not imply those generated artifacts were all uploaded.

## Latest observations and interrupted work

The ordinary-outbound V2 frozen course corrected its same-callback Wait/seat setup race and observed actual initial-departure readiness. The same original follower activity passed 152.760 m goal-axis progress /37 powered intervals and independent net route approach 100.042 m. Whole-run spacing failed at 303.235 m; separation safety cleared following and the 90-world-second arrival window expired. No arrival hold, explicit Hold/Resume, unloading, parking, regroup or return was reached. Native 100 loading conserved supplies. The natural 30.0157-world-second close retained 19 gameplay /0 post-result /59 shutdown diagnostics.

The corrected diagnosis uses 201 regular paired vehicle samples. Renewed gap growth is associated with the initial road join, but under-speed continues after alignment; the native cause remains unisolated. The earlier sparse path-pair timeline is retained as superseded evidence. See the development checkout's `docs/experiments/ordinary-outbound-bay-v2-live-result.md`.

The next private road-start control moved only one authored world layer, preserving all script/production bytes, later bends and the endpoint. Lead/follower separation is 21.112 m and lead-to-endpoint distance is 384.550 m, retaining the 350 m guard. Pack data SHA-256 is `B279929DBDA1F2836E9124AF5A071C855E654E319C00B8561AB87EBD53D298B3`; source is `D:/ReforgerAgentBuilds/ordinary-road-start-join-v1/frozen-source/source`, runtime parent is `D:/ReforgerAgentBuilds/ordinary-road-start-join-v1/frozen-addons`, run evidence is `D:/ReforgerAgentRuns/ordinary-road-start-join-v1`. It launched once and is being closed by its owner for Marco's takeover. **Treat it as interrupted evidence unless the owner's final record proves a scenario terminal result; do not turn a user-requested exit into an acceptance result.** Do not repeat it automatically on resume.

## Untimed player test

Use the ordinary **V8** player scene, not the scripted diagnostic world. It has no automatic pilot/recruitment/supply transfers. The preserved runtime is:

`C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/everon-journey-scene-v8/ConvoyFollower_5A5FB20BD40C7C70`

Matching hashes: data `727B39B942A6325FB343BCA744C5C63CEED4E25B5772F614193CB4347B9BCD55`; RDB `569AA9F3FB2AC691E383FD9735EDEB70FAB874C6D21C9390A297251A3F3CF175`; project `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579`.

When Marco requests the player scene and the owned diagnostic has closed:

```powershell
Set-Location -LiteralPath C:/Users/marco/.codex/worktrees/175a/REFORGER
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/everon-journey-manual-v8 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir C:/Users/marco/.codex/worktrees/175a/REFORGER/.cache/player-addons/everon-journey-scene-v8 --keep-open --expect-game --execute
```

`--keep-open` leaves the player session untimed. The D: profile/log location avoids the observed C: free-space preflight refusal. The helper sets the game's executable working directory. GAME startup is not evidence of any completed player action.

Choose the **Convoy supply journey - Regina daylight staging** US spawn and leave the Game Master editor. The front M923 is the player lead; the original driver/follower truck is behind. Source storage is near XZ 7225/2430; destination near XZ 7172/2907. Follow the roads north toward XZ 7174/2917. Exact six-step instructions are in the development checkout's `docs/convoy-regina-supply-journey-day.md`.

Useful feedback, in order:

1. Recruit the driver and inspect boarding. Test F8 and the contextual **Convoy commands** opener on foot and from the stopped lead seat. Open, cancel and close; verify walking, steering and throttle respond afterward. Report unclear labels, clipped instructions or unavailable-action reasons.
2. Load 100 into each truck through native rear actions and cancel transfers. Expected source/lead/follower/destination counts are **1600/100/100/0**. Check actual counts rather than estimating from appearance.
3. Drive with the original assignment intact. Note where braking, turns, lag or a blocked status first appear; include speed and the displayed reason. Reliable spacing remains unproved.
4. At the destination, stop the lead and choose **Unload bay → Set bay / Hold queue**. Wait for a seated Hold, unload the lead's 100 and move it clear. Then **Admit next truck**, observe its stopped arrival and unload its 100. Expected final counts are **1600/0/0/200**.
5. Try the unit's **Pull off / regroup**, inspect physical parking, then **Regroup for return** and lead the original truck back to staging. If blocked, preserve the truck/cargo/assignment and capture the reason; do not repeatedly move the bay or dismiss/recruit a replacement to manufacture success.

A short recording with commentary is useful, especially around the first unclear command or driving failure. Native radial instructions were inspected in static tests; normal F8/cancel/restored controls and this full ordinary journey are unproved. Initial source access was independently qualified for each truck; simultaneous loading and destination reach remain pending. Focused bay unloading/parking worked in some separate runs, but parking repeatability and return still fail or remain unproved. No multiplayer, full-trip, demonstration-video or release-ready claim is made.

## Resume boundary

Preserve all frozen builds, failed runs, raw full logs and first-observed snapshots. Keep whole-link 60 m spacing, continuous goal/net 100 m /three powered intervals, exact 180 s /2 m arrival, real 30 s Hold and fresh 20 m /three-powered Resume gates. Do not reopen the failed scripted-owner control-write matrix, cap sweeps, stock-Follow substitution or initial-guard bypass. Review the interrupted road-start record and Marco's player feedback before choosing another implementation or run. Desktop input automation remains unavailable; native screenshots provide image evidence, not normal input or video proof.
