# Marco takeover checkpoint — September 30, 2026

Development is paused at Marco's explicit request. No autonomous features, tests, launches or goal continuation until he resumes. The overall control chat is `01a0f3e6-7755-7ae3-9e16-8d88e57a6a4b`. Do not mark the development goal complete. The three historical subagents are already completed; there are no active child agents. No convoy-owned recurring automation was found; the unrelated YouTube automation is already paused and was not changed.

## GitHub checkpoint

Addon worktree: `C:/Users/marco/.codex/worktrees/175a/REFORGER`, branch `codex/convoy-player-trip-20260929`, remote `https://github.com/MarcoBetti1/reforger-convoy-follower.git`. Source/docs/evidence checkpoint commit **ac55826be5c17f8a4e6c1b1c1c7c76b082bcf231** was pushed, and git ls-remote returned that exact SHA for this branch. This note's final receipt is a documentation follow-up commit; the source checkpoint remains an ancestor. No force push or merge occurred. Main checkout `C:/Users/marco/Desktop/REFORGER` was not committed or reset by this chat.

Generic harness notes were committed in a separate D: worktree on branch `codex/convoy-handoff-20260930`, commit **c7b234a6602fb124bae3c0eaa44a5c9384af17a1**. Remote `https://github.com/MarcoBetti1/reforger-agent-harness.git` returned that exact SHA for the branch. Both `docs/operations-playbook.md` and `docs/client-window-options.md` are backed up. The shared harness main checkout retains its existing dirty copies; no concurrent changes were discarded. Isolated checkpoint worktree: `D:/ReforgerAgentBuilds/harness-handoff-20260930`.

## Machine state and interrupted run

This chat's isolated road-start client launched at **14:59:20 Central** under the reviewed frozen authorization. Marco's takeover request superseded it. The last console line is **14:59:34.051**: world/course initialization occurred, but no cargo transfer, paced world-time, ready=true, driving, arrival, terminal result or native image was recorded. No clock/focus cause is established.

Normal close requested **15:02:02.663 Central** did not complete after bounded observation. Only verified owned PID9268 was stopped; client launcher43749 and watcher83530 are closed. Eight observed pre-shutdown errors remain; shutdown is censored and the declared natural30world close did not occur. No terminal prefix was fabricated. Full24676byte console/user-abort copy SHA **E909B7676C03ADC7061F51B8AA30CCCDFCCC0F4A1AF5B8CAC286A785243C2227**. Post-stop616source/238packedtext/trio match. See [the interruption record](experiments/ordinary-road-start-join-v1-interrupted.md). The one launch is **interrupted, no scenario result**; do not label it a road-start failure or PASS, and do not automatically replay it.

Game and Workbench processes owned by this chat are closed. No player session was launched. The control chat will wait for Marco's request before starting one.

## Exact human-test candidate and launch

Use the **ordinary manual V8** pack, not the private road-start observer pack. It contains the provisional Everon Regina supply day scene, current instructions/identity guards and ordinary convoy control path. Data **727B39B942A6325FB343BCA744C5C63CEED4E25B5772F614193CB4347B9BCD55**, RDB **569AA9F3FB2AC691E383FD9735EDEB70FAB874C6D21C9390A297251A3F3CF175**, project **1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579**. All three rehashed unchanged after preparation. GUID `5A5FB20BD40C7C70`, one child directory in the runtime parent.

When Marco asks to launch, run from this worktree. It is untimed and leaves the game open until normal exit or Ctrl+C. New profile/logs go to D: because C: was below the existing2GiB guard; no old evidence was deleted.

```powershell
npm run client:run -- --profile D:/ReforgerAgentRuns/everon-journey-manual-v8/profile --run-dir D:/ReforgerAgentRuns/everon-journey-manual-v8 --world Worlds/Showcase/ConvoyFollower_Everon_ReginaSupplyDay_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir .cache/player-addons/everon-journey-scene-v8 --keep-open --expect-game --execute
```

The helper supplies the executable working directory. `--expect-game` verifies startup, not gameplay. Runtime pack: `.cache/player-addons/everon-journey-scene-v8/ConvoyFollower_5A5FB20BD40C7C70`; original frozen pack `.cache/frozen-addons/everon-journey-scene-v8`; exact source `.cache/frozen-source/everon-journey-scene-v8/source`. [Scene guide](convoy-regina-supply-journey-day.md) and [player guide](convoy-player-guide.md) describe the native actions.

Start with one follower. Check ordinary F8 opening on foot and in the lead seat, reason/instruction text, cancel/close and restored walking/driving input. Recruit and inspect the original driver boarding. Use the actual native rear Load supplies interaction, then drive the outbound road, stop and observe arrival, issue Hold/Resume, admit the next truck to the unloading bay, unload through the native rear action, command parking, then Regroup and drive home past the parked line. Preserve actual before/after quantities and logs. Ask Marco where a truck first hesitates, brakes/reverses, loses its predecessor or rejects a command; distinguish rejected availability reasons from accepted-but-incomplete behavior. A short driving/command observation is useful feedback even if the full trip fails.

## Verified behavior and remaining failures

Static native100 load/cancel/unload-back-to-source checks separately passed for each initial truck; simultaneous loading/destination delivery in this ordinary scene remains open. Scripted native radial V3 renders actual instruction/status/translated-truck pixels and closes natively; normal F8/cancel/restored input remains unverified. Focused bay runs admitted/unloaded100, parked and held30world, but parking repeatability is mixed and return after a completed ordinary outbound has not passed.

Closed ordinary-outbound V2 fixed the private selected-Wait startup defect, observed actual core ready=true and earned strict continuous goal-axis152.76m /37powered plus independent net100.042m (reconstructed100.0336468m). Overall **FAIL**:60m breached at32.567drive world seconds, peak303.235m, LOST then90.165world owned-arrival timeout. No Hold/Resume/bay/delivery/return occurred. All266full-lifecycle ledgers conserve1800;19gameplay/0post-result/59shutdown errors remain after natural30.0157world close. Renewed separation around the initial road join continues as underpace after alignment; native formation demand/braking/gear/camera overhead causes remain unisolated. [V2 result](experiments/ordinary-outbound-bay-v2-live-result.md) preserves both sparse preliminary and corrected201-pair timelines.

No full player-led loop, repeatable one/two/three-truck driving, sustained return to original staging, normal-controls demonstration video or release readiness PASS. Preserve current turn code and safety guards; do not reopen closed cap/backend/control-write/retention matrices automatically.

The latest private diagnostic is [road-start join-removal V1](experiments/ordinary-road-start-join-v1-preparation.md), data hash B279929DBDA1F2836E9124AF5A071C855E654E319C00B8561AB87EBD53D298B3, frozen parent `D:/ReforgerAgentBuilds/ordinary-road-start-join-v1/frozen-addons`. It changes only one authored layer from V2;616source/238text/all5compile/pack/trio verify. Lead7173.61/143.73/2532.75, follower7175.97/142.211/2511.77,21.112m gap and384.550m endpoint distance. Later bends remain. Relocated source rear access, settling and simultaneous clearance are unverified; the original native100/cancel3s barrier must precede driving. This private pack is not the human-test default.

## Backup coverage and irreplaceable local evidence

**The source/docs Git push backs up current authored addon code, scenario resources, this handoff, exact source manifests/overlays and preparation/run review reports.** `docs/experiments/archives/backup-inventory.json` lists copied files and original hashes. The adjacent reconstruction script validates the complete607-file manual source or616-file V1/V2/road-start sets before writing a new directory; it accounts for Git text line endings only when the exact original SHA matches. Preparation helpers retain original local paths/imports for audit, not blind execution.

**Raw .cache and D: artifacts are not all backed up by that Git commit.** Frozen data.pak/runtime packs, raw full/terminal/crash logs, native screenshots and videos remain local except for the separately verified draft backup coverage below. No profile/auth data is included. Draft GitHub release400404255/tagconvoy-takeover-2026-09-30 is now independently verified through the GitHub API: frozen-source/playerV8 ZIP227893930bytes SHA4F2022A6D1F914D602455E7A3BFB48309DDFFDEBBB86A6FEFBD4E7AEE8571421; selected test-evidence ZIP149328365bytes SHAC5B356D6790FD6960685712B66EE6730C27B887C805697E0BFDCF694E908656E. The control receipt describes seven worktree frozen-source sets plus exact ordinaryV8 trio, and571selected worktree/D: run files excluding profiles/downloads/savegames. [Verified draft backup](https://github.com/MarcoBetti1/reforger-convoy-follower/releases) is private to authorized repository users while draft. It is separate from this source/report Git commit; it is not a blanket backup of every .cache or D: artifact. Remote asset digests are in docs/experiments/archives/verified-draft-backup-receipt.json.

Preserve these local roots:

- `D:/ReforgerAgentRuns/ordinary-outbound-bay-v1` and `ordinary-outbound-bay-v2`: original full/terminal logs, native BMPs, lossless PNGs, independent source/cargo/visual/timeline reports.
- `D:/ReforgerAgentRuns/ordinary-road-start-join-v1`: original interrupted full/user-abort logs and post-stop verification; no native image or terminal result.
- `D:/ReforgerAgentBuilds/ordinary-outbound-bay-v1`, `ordinary-outbound-bay-v2`, `ordinary-road-start-join-v1`: exact source, compiled frozen trios and Workbench evidence.
- `.cache/frozen-addons`, `.cache/frozen-source`, `.cache/player-addons`, `.cache/everon-journey-scene-v8`: ordinaryV8 and earlier candidate packs/source/manifests.
- `.cache/client/runs`, `.cache/test-videos`, `.cache/workbench-runs` and other dated experiment roots: earlier full logs, terminal prefixes, crash/dump artifacts, source/provenance and demonstrations. The file-backed recording inventory at docs/experiments/archives/local-recording-inventory.json lists221actual videos totaling68412459812bytes across this worktree, the shared main checkout and D:. These are explicitly excluded from this Git source checkpoint and no complete video-archive upload is verified. Earlier development recordings are not a completed ordinary full-trip video.

Resume only after Marco explicitly resumes. Read current validation, this handoff and the preserved result/constraint records first. Resolve the human feedback or next agreed bounded diagnostic while preserving prior failures and unchanged gates.
