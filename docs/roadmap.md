# Roadmap

**Execution active after explicit user continuation.** The documentation-only setup is complete and left the saved goal untouched. The subsequent explicit continuation authorizes development and validation (`get_goal` updatedAt `1790418546`). The brief's embedded start/resume wording was not that authorization. Preserve the full brief, newer source and evidence.

## Current product direction — September 26, 2026

The full [Convoy Follower development brief](convoy-development-brief.md) supersedes the bounded overnight direction and the original harness plan below. Use [the adaptive product roadmap](convoy-development-reset.md) for milestone sequencing, [the current validation record](convoy-validation-current.md) for observed results, and [the navigation plan](convoy-navigation-release-plan.md) for driving experiments and release gates.

The complete brief is saved unchanged and reconciled with the existing roadmap. The completed setup left the saved goal untouched; the subsequent explicit continuation now authorizes work toward the milestones below. Preserve source changes and evidence beyond the older pause checkpoint.

**Latest reconciliation:** preserve mod checkpoint `c298330`, harness `d18d920`, and the later default-off stable-waypoint source candidate. The candidate already passed five-configuration validation and packaging (`E765E6B4...`), then failed its two-follower physical/strict comparison: peak gaps 169.224 / 131.836 m, worse than the baseline's 123.379 / 88.5554 m. Its 180.363-second zero-drift hold passed, but forced termination left normal cleanup unobserved; nine runtime errors remain. It stays off by default and is not promoted. Saved native-lifecycle research proposes an isolated persistent vehicle-follow comparison as the next experiment; no such prototype is physically validated. Ordinary input also remains unresolved. Use the [validation record](convoy-validation-current.md), [comparison note](convoy-stable-waypoint-comparison.md) and [input investigation](convoy-input-investigation.md) for provenance and proposed next steps. Historical proposed diagnostics below do not require repeating completed work.

The moving-restart review is complete: spacing passed its bounded gate, followed by later follower creep. The two saved arrival-zero comparisons record peak gaps of 47.3505 m and 51.4730 m, each with 180 seconds without follower drift. Both strict full runs fail on retained runtime/shutdown errors; this is repeat evidence for one bounded fixture, not general release readiness. The separate explicit Hold report is complete (physical PASS, strict full-run FAIL). Preserve the compiled control-handover/Resume-status candidates, newer status-fixture artifacts, and all earlier failed comparisons. Resume-status v2 is a reviewed seat-transfer fixture failure. V3 now passed its targeted authoritative waiting/progress/completion episode, but the whole run failed spacing at 67.446 m and strict error gates. Its exact scope is recorded separately from human input and general driving. Use the validation record for exact package provenance and limits; the brief's older experiment status must not reset this progress.

The subsequent paced-two-follower course failed the 60 m gate with a 230.336 m peak; it did not reach its 180-second observation. Its original identities and powered travel were retained, and wheel samples establish mixed dirt/grass/asphalt on a mapped road. The three-follower course remains unrun. The later panel correction rendered correctly without slot errors, while clicks after recovered activation produced no order/map-close effect. Both runs ended by timer with normal lifecycle unobserved; retain the paced run's nine runtime errors and panel run's eleven. Preserve these separate outcomes and the completed reports in the validation record when selecting the next comparison.

The new combined departure/panel candidate `C657779A...` passed five-configuration validation and packaging. It includes initial-departure Wait, guarded panel event resolution, native danger telemetry and a one-follower paced world. The completed two-follower run fails spacing at 121.072 m, while all trucks retain identities and hold without measured drift for 180.513 seconds. Normal cleanup still retains nine runtime and 64 shutdown errors. Panel v3 produced no widget callbacks, orders or map closure, with 11 runtime and 64 shutdown errors after normal closure; the resolver was never exercised. Treat the remaining new behaviors as compiled candidates until their separate physical gates are observed. [Focused courses](convoy-feature-courses.md) define those checks.

The later read-only input/path observer package `8C58BE9E...` passed all five configurations and packaging. Its completed raw-input panel comparison still establishes no ordinary-input pass: click/short-drag/Escape yielded zero sampled raw changes, callbacks, orders or map closure, while 447 post-map samples retained the original pilot. Normal closure retains 11 runtime and 62 shutdown errors. Bindings and handlers were present; the shared delivery/focus cause remains unresolved. Obtain positive ordinary-input evidence before attributing failure to production panel code. Zero getters do not prove retail no-op behavior or absent physical input; preserve the full results and broader driving/supply gates.

The subsequent map-free input world (`C54DF7B7...`) also showed no raw key/button or mapped-action response to Up/M/click/Up, despite original pilot identity, inactive AI, enabled controls, no native menu/editor and active mappings. A late cursor change is separate evidence; key/button runtime sensitivity is still uncalibrated. Normal closure retains nine runtime and 64 shutdown errors. This rules out map-open suppression as a sufficient explanation for those attempts, without establishing the shared input cause or an OSK-removal comparison. The next input work needs positive ordinary-input evidence rather than another ungrounded production-panel change.

The playable milestones are dependable movement/hold/restart; a complete supply trip through ordinary player controls; multiple followers and interruptions; complete assignment, individual commands, arrival/regroup/return; and a polished, supported release candidate covering feedback, multiplayer, configuration, performance, and distribution. The first supply trip is an intermediate milestone. Preserve the predecessor chain, one radio spokesperson, and five-follower cap excluding the player's lead vehicle. Autonomous live verification, actual cargo transfer, human input restoration, multiplayer evidence, and a real-map demonstration remain required.

Generic tools and operating lessons belong in the independent [Reforger Agent Harness repository](https://github.com/MarcoBetti1/reforger-agent-harness); addon source, assets, worlds, convoy reporters, and gameplay evidence belong here. Raven research is closed.

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
