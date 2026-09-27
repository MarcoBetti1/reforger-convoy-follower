# Roadmap

**Current execution status — September 27, 2026: active development.** The documentation-only setup turn is complete. The user's subsequent active-goal continuation authorizes implementation and live testing under the full brief. Preserve earlier development and evidence.

## Current product direction — September 27, 2026

The full [Convoy Follower development brief](convoy-development-brief.md) supersedes the bounded overnight direction and the original harness plan below. Use [the adaptive product roadmap](convoy-development-reset.md) for milestone sequencing, [the current validation record](convoy-validation-current.md) for observed results, and [the navigation plan](convoy-navigation-release-plan.md) for driving experiments and release gates.

The complete brief is saved unchanged and reconciled with the existing roadmap. Its older restart commits describe historical context; preserve source changes and evidence beyond that checkpoint. The latest emphasis on a useful, demonstrable mod determines the resumed development priority.

## Current development boundary

The latest focused command-recovery course physically passed: a deliberately failed authored order, real **30.0995 s Hold without drift**, then fresh powered travel by both original drivers (**21.3237/22.0079 m**, four powered intervals each). Peak spacing stayed **58.8781 m**; diagnostic errors remain **9/0/64**. The three reviewed product changes are now canonical and passed all-five validation/packaging (`F3CD0563…98CC9B6`). The established complete supply course is running next, with the pending arrival fix kept private. This is explicit recovery evidence; native path planning, ordinary panel input and full-trip repeatability remain separate. See the [paired report](experiments/explicit-retry-2truck-v2.md) and current validation for exact provenance.

That [complete supply attempt](experiments/cooperative-supply-recovery-v1.md) is now closed: both trucks loaded100, but a natural tail movement-order failure stopped the course before delivery. All190 ledger records conserved1800; no explicit retry was requested by the unchanged observer. Next, test one deliberate owner Hold/Resume response to the real failure and continue to the original delivery goal. Preserve the original failure as a recovered episode, not clean first-attempt success; retain cargo, arrival and final fresh-departure checks. The arrival candidate remains private and unexercised.

The tested ordinary controller is integrated into US/USSR driver prefabs. US one- and two-follower travel and sustained arrival passed at a declared 20 km/h lead pace. Preserve the completed one-follower load/transport/Hold/delivery of 100 supplies/powered Resume baseline. The early two-loaded-truck regression delivered **200 supplies** to a distinct destination, retained both assignments, completed **180.313 s arrival** and **48.1325 s explicit Hold** with zero measured drift. Its recorded result remains **FAIL**: Unit Two peaked at **81.625 m** against the 60 m spacing gate, then production Resume failed with `measured_step_spans_multiple_vertices` before fresh movement. Nine gameplay and 64 shutdown errors remain. See [current validation](convoy-validation-current.md) for the later results and preserved failed controls.

The two-truck regression resources are canonical. The earlier [V3 comparison](experiments/loaded-straight-trip-v3.md) failed spacing and fresh restart gates; the subsequent [Road81 trip](experiments/road81-arrival-trip-v1.md) earned both Resume sequences but timed out on simultaneous qualification. The [integrated Road81 loop](experiments/road81-integrated-trip-v1.md) now completes all native supply/Hold/Resume stages with the reviewed per-member fixture correction. It delivered **200**, held arrival **180.279 s** and explicit Hold **51.1149 s** with zero drift, and earned fresh head **35.1900 m /7 powered intervals** and tail **21.2586 m /8**. Overall FAIL remains: **172.483 m head /44.9963 m rear** spacing and **9 /0 /62** errors. Preserve the earlier raw results. Ordinary input, repeatability, Soviet behavior and multiplayer remain unproved.

Owner-passenger Resume, the Supply Day source placement, selected-Reboard and the five tested driving files are canonical, with exact owned-Wait fixes preserved. The focused held-Reboard regression passed physically. Canonical package **`1E466B71872F9F339FAE99FB1AD63D99D84075B05D6D015038956B23E5FA440A`** passed all-five validation/packaging. The complete supply-loop package **`0813A7E99FE8861DF6F3EF71791FC8CA6D62FB4642881FFCBE84AB797069E77C`** preserves all nine gameplay/Reboard hashes and adds six private fixture overlays, including qualification retention and a steeper camera. It is a measured integration regression, not a rerun of the exact canonical package or a Reboard invocation during travel. Earlier route/lifecycle failures remain historical controls.

### Immediate playable milestone

**Current next step:** close the player-control recovery gap exposed by the cooperative supply runs. The [V1](experiments/cooperative-cohesion-trip-v1.md) and [V2](experiments/cooperative-cohesion-trip-v2.md) runs measured closer travel with early leader advice, but failed before delivery; they do not replace the preserved complete supply-loop milestone. The joined-tail Hold/Resume path bypasses the existing native-request retry. Reuse its safe preflight and verify retained assignment and fresh powered movement in a clearly labeled injected-failure command test, then return to the complete supply trip. Six feedback files and five ordinary Road81 scene resources are now integrated and passed all-five validation/packaging; unexercised arrival and fixture changes stay private. Current validation owns exact evidence and limits, including unresolved ordinary input.

The [matched one-follower Road81 comparison](experiments/head-guide-road81-v1.md) passed travel/arrival but failed spacing at **128.812 /100.092 m**. The subsequent [early-prejoin offer](experiments/head-guide-prejoin-approach-v1.md) still failed at **97.8129 m**, with only **0.383 s** improvement from guide creation to genuine join. Its weak effect does not justify promotion or more startup tuning. Head-guide arrival/Hold/Resume remains unproved; these experiments remain private.

The [native precision comparison](experiments/direct-head-native5-v1.md) is closed: 5 m still failed at **118.396 m** and barely changed sustained speed (**11.16 to 11.33 km/h**). The [continuous-position proxy comparison](experiments/direct-head-proxy-v1.md) also failed at **125.481 m**, with sustained speed **11.29 km/h** and **180.23 s zero-drift arrival**. Its all-five validation/packaging, 598 sources/three deployments and natural closure are verified; **9/0/64** errors remain. Both candidates remain unpromoted; neither resolved the direct head's sustained lag.

The [15 m held-target / native20 comparison](experiments/direct-head-sampled-proxy-v1.md) now shows useful improvement: matched-window speed **15.90 versus 11.29 km/h** and peak **79.1669 versus 125.481 m**. It retains **251.197 m /35 powered follower intervals** and **180.413 s /180 zero-drift arrival samples**, with 598 sources/three deployments verified and natural closure. It still fails the unchanged 60 m gate and retains **9/0/64** errors. This single run remains private; cargo and explicit Hold/Resume under this policy are unproved.

The [held15 + native5 interaction](experiments/direct-head-sampled-native5-v1.md) is closed: speed improved to **17.55 versus 15.90 km/h**, but **66.1315 m** peak spacing still fails. Original follower progress was **259.4206 m /31 powered intervals**, with **180.346 s /180 zero-drift arrival samples** and **9/0/62** errors. All-five validation/packaging, 598 sources/three deployments and natural closure are verified. The candidate remains private; this travel/arrival comparison proves no cargo or explicit Hold/Resume under its policy.

**The sampled-head whole-trip comparison closed early.** The [integration report](experiments/sampled-head-supply-integration-v1.md) verifies **100 natively loaded into each truck**, 109 conserved ledger rows and final **1,600/100/100/0** counts. Unit2 failed `prejoin_window_history_changed` before genuine join; arrival, Hold, delivery and Resume were not reached. The early **44.8457 m** peak is not a full spacing pass. All-five validation/packaging, **587 sources/three deployments**, terminal snapshot and natural closure are verified on package `F683F18D…704B0300`; **9/0/64** errors remain. The candidate stays private and unpromoted.

**V2 delivered, then failed restart.** The [repeat supply trip](experiments/sampled-head-supply-integration-v2.md) genuinely joined the tail, delivered **200**, retained **459** conserved ledger rows and completed **180.446 s arrival /51.1162 s Hold** with zero drift. Neither follower qualified Resume; the tail blocked on `reverse_physical_context_unavailable`. Peaks **106.677 m head /72.0678 m tail** both fail: better head spacing than the earlier 172.483 m, worse tail spacing than 44.9963 m. Entry succeeded without a retention-branch event. All-five validation/packaging, **587 sources/three deployments** attribution and natural closure are verified on pack `9B0D072E…419147F`; **9/0/64** errors remain. No promotion.

**V3 delivered and physically restarted; declared gates still fail.** The [pending-guide comparison](experiments/sampled-head-supply-integration-v3.md) delivered **200**, conserved **531** ledger rows and completed **180.427 s arrival /51.1648 s Hold** with zero drift. Its observation branch ran and no trail block occurred. Actual net restart was **54.9914 m head /53.7483 m tail**; head qualified at a best **50.888123 m /11 powered**, but tail's moving sequence earned only **16.51392 m /6** before a legitimate real-predecessor 10 m approach. Preserve the raw timeout and **111.888/69.3399 m** spacing failures. All-five validation/packaging, **587 sources/three deployments** attribution and natural closure verified pack `60BE1963…53FA99D`; **9/0/64** errors remain. The [76-second game-only demo](../.cache/test-videos/sampled-head-supply-integration-v3-demo.mp4) is rendered with automation/failed-gate captions and sampled contact-sheet/end-card inspection; no full-playback or promotion claim.

**V4 completed the supply/Hold/Resume loop.** The [closed repeat](experiments/sampled-head-supply-integration-v4.md) delivered **200**, held arrival **180.426 s** and explicit Hold **51.0648 s** without drift, then independently qualified fresh head **20.853023 m /5 powered intervals** and tail **25.206492 m /9**. Both earned that result on their moving sequences before the handoff; V4's new direct-leg admission remains unexercised. Overall FAIL retains **128.138/75.0655 m** spacing and **9/0/64** errors. All-five validation/packaging, 587-source/three-file deployment attribution, terminal snapshot and natural closure are verified; no experimental policy is promoted.

**Stop isolated parameter tuning; integrate useful fixes and improve cohesion.** The pending-guide observation repair is now canonical and passed all-five validation/packaging (`5756BF25…5A7AB84`); it excludes the unexercised V2 prejoin-retention branch, sampled-head proxy and V4 observer changes. Its [closed regression](experiments/canonical-pending-guide-trip-v1.md), pack `0CAC7F85…20D00BA`, exercised the branch, delivered 200, and held arrival180.51s/Hold51.1982s without drift. Raw FAIL retains head169.795m/tail42.5945m spacing and a stale retired-action fixture rejection during Resume. Later movement is separate, not pass credit. No more V4 qualification iterations or precision/proxy parameter sweeps. Prepare early leader-only cohesion advice and measured gap in the authoritative panel; retain assessment during an ordinary seated gathering stop, while explicit Hold/maneuvers remain separate. A later cooperative-lead comparison must consume that same advice without altering follower control or the 60 m gate, and must be identified as cooperative driving rather than a sustained-speed fix. Include only a narrowly guarded deferred-readiness correction for the observed retired action; do not rerun a whole trip merely to change metrics. Preserve the complete trip while continuing toward normal player controls.

Return useful findings to the integrated supply workflow. The completed loop has a rendered and delivered [90-second development demo](../.cache/test-videos/road81-integrated-trip-v1-demo.mp4), with sampled contact-sheet/end-card review. Its raw recording has five sampled views, not full playback. Keep spacing failures and automation limits visible. The per-member qualification correction is live-proved but remains a private fixture.

The latest user direction is to zoom out and deliver a working mod that can be shown in use. Concentrate the next iteration on **recruit/assign → load → travel → stop/Hold → unload → Resume**, with the original trucks and membership preserved. Record that complete journey, state which interactions are automated, and carry demonstrated improvements into ordinary gameplay.

Prioritize a smaller fix when it blocks that journey. Use the established fixtures to diagnose it, then return to the full trip; avoid extending diagnostic machinery or adding optional polish without a specific product benefit. Preserve the harder junction and three-follower failures as regression requirements. A successful straight-road demonstration is progress toward those requirements, not a replacement for them.

The playable milestones remain: dependable movement/hold/restart; an actual supply trip through normal player controls; multiple followers and interruptions; assignment, individual commands, arrival/regroup/return; and feedback, multiplayer, configuration, performance and distribution. Keep one spokesperson and the five-follower cap, excluding the player lead. A first supply demonstration is a meaningful milestone within the full brief.

Generic tools and lessons belong in the independent [Reforger Agent Harness repository](https://github.com/MarcoBetti1/reforger-agent-harness), now pushed at **`4641367`** with reusable loopback audio capture and prior operating lessons. Addon scripts, assets, worlds and evidence belong here. Preserve the integrated controller and frozen comparisons; [current validation](convoy-validation-current.md) owns source checkpoints. Workshop publication remains separate.

## Historical harness plan — retained for context

The sections below record the original workspace plan. Their user-led testing scope, Raven handoff, repository arrangement, and unfinished setup tasks are historical, not current acceptance criteria or new work orders.

### Completed baseline: repeatable operations

- Acceptance scope from the user: Enfusion Workbench is the main automation surface. Cover world launch, private hosting, and a small Game Master smoke test. The user will perform most first-person gameplay testing, so raw in-world keyboard control is not a harness completion gate.
- Keep the verified Reforger UI and command-line procedure in [operations-playbook.md](operations-playbook.md).
- Check the machine with `npm run cli -- doctor` before each test session.
- Use isolated mod sets and scenario IDs for truck and helicopter comparisons.
- Validate scripts and pack addons through the Workbench helper once an addon exists.
- Dedicated server fetch and loopback binding are verified for Raven with config mode. Keep using the guarded runner; the quick `-server` world path ignored loopback flags in a smoke test.
- `mod:fetch` wraps that private server path into one command, keeps a reusable cache, and verified a fresh Raven package in a live run.
- A clean client profile launches Game Master Arland directly with `--world`, bypassing the unreliable splash. A client can load Raven directly from the server's addon cache. Automatic client join to the local server timed out during authentication even in a no-mods vanilla control; the cause remains unverified.
- A Raven-loaded Game Master run accepted mouse-driven entity placement and logged the spawned squad. Raven logistics did not bootstrap there. In a later isolated profile, a real in-game Workshop download made Raven selectable in Host Mods; a private player-hosted Conflict Arland run reached `GAME` and Raven logistics bootstrap. No completed delivery was verified. The interactive menu gate remains unreliable for desktop input, so use direct world launch for local Game Master tests.
- `conflict:host` now creates and runs a private Conflict Arland dedicated server with selected Workshop mods in one command. The previous automatic local join timed out during authentication, so manual in-game Direct Connect remains a separate acceptance test. The working player-hosted path currently provides the live Raven handoff.
- Choose the next addon scope with the user. For any logistics comparison, record actual setup time, supply counts, failure recovery, and logs before treating a delivery as proven.

### Phase 1: Foundations

- Keep this repository as the control plane for the helper.
- Initialize Git and GitHub.
- Store design notes, prompts, and helper code here.
- Keep actual mod source in separate repos or separate folders with their own Git history.

### Phase 2: Read-only mod intelligence

- Finish `reforger-mcp` read-only tools.
- Point it at:
  - one local mod root
  - the official samples repo
  - the local Workbench documentation
- Validate that the agent can answer:
  - what files belong to the mod
  - where scripts live
  - which sample is closest
  - which file probably needs editing for a requested change

### Phase 3: Safe editing loop

- Add guarded write tools for text files only.
- Require edits to stay inside configured roots.
- Require the agent to report exact file paths and a diff summary.
- Use one branch per requested mod change.

### Phase 4: Build and test automation

- Add wrappers for Workbench CLI or plugin-driven tasks.
- Add logs and failure summaries back into the agent context.
- Start with dry-run validation and packaging, not direct publishing.

### Phase 5: CI/CD

- CI for this helper repo:
  - install
  - typecheck
  - test
  - build
- CI later for mod repos:
  - structural validation
  - scripted checks
  - packaging steps if practical
- CD should remain human-approved until the workflow is stable.

### Phase 6: First real project

- Create or clone a small learning mod.
- Use the agent to make one change at a time.
- Test locally after every change.
- Build a library of successful prompts and failure cases.
