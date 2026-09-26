# Roadmap

**Current execution status — September 26, 2026: active development.** The user has resumed the full revised goal (`updatedAt: 1790437454`). The preceding documentation-only setup is complete; its restrictions below are historical. Preserve newer source and evidence, finish v2 evidence closure, and investigate continuous Resume separately from multi-truck route guidance. The previous goal turn made documentation progress.

## Current product direction — September 26, 2026

The full [Convoy Follower development brief](convoy-development-brief.md) supersedes the bounded overnight direction and the original harness plan below. Use [the adaptive product roadmap](convoy-development-reset.md) for milestone sequencing, [the current validation record](convoy-validation-current.md) for observed results, and [the navigation plan](convoy-navigation-release-plan.md) for driving experiments and release gates.

The complete brief is saved unchanged and reconciled with the existing roadmap. This setup leaves the saved goal untouched; work toward the milestones below awaits the user's update and resumption. Preserve source changes and evidence beyond the older pause checkpoint.

## Current development boundary

Preserve mod checkpoint `506d631`, its earlier `4f56f64` and `3737f08` history, harness checkpoint `0d960a0` (after `419dba5`), and all newer local source and frozen evidence. The [current validation record](convoy-validation-current.md) owns exact results and hashes; the historical commits quoted in the brief must not reset that progress.

The [original Follow graph experiment](convoy-original-follow-experiment.md) has a one-follower physical PASS / strict full-run FAIL. Its two-follower run has closed normally with a known second-link spacing failure; final independent analysis confirms physical and strict full-run FAIL. It remains private and unpromoted: arrival reversals, interrupted-order/Resume behavior, copied-comparator distribution, and broader driving still require work. Earlier failed entity-follow, trail-guide and pacing comparisons remain preserved in validation and their experiment records.

The newer private command repair, parked-lead policy and Hold/Resume fixture have compiled and run as `FA913F7A...`. Their baseline passed its bounded travel and 180-second hold checks, but actual Hold released the arrival Wait and failed physically; Resume was never reached. Preserve natural closure, nine runtime / 62 shutdown errors, and the distinction between observed normal-revocation repair and unobserved parked-lead admission. The tail-guide movement candidate remains cache-only, reviewed but uncompiled and unrun.

The newer explicit-Hold controller is already integrated privately and compiled in `59F79906...`. Saved v2 evidence passes baseline travel/180-second hold and explicit Hold for 30.0662 seconds, then fails the continuous Resume gate when `ARRIVAL_RESUMED` replaces a healthy activity. Its final independent report confirms natural closure and finalized recording; nine runtime and 64 shutdown errors remain. The authored-source export is a separate cache-only draft awaiting review/build; neither it nor the private controller is production-promoted.

Queued after the user's goal update and explicit resumption: retain the finalized v2 evidence, then investigate preserving a valid owned activity across the arrival-to-following transition without relaxing restart or ownership gates. Separately use the finalized two-follower spacing diagnosis and staged tail-guide candidate for a discriminating movement comparison. Reconcile the tail candidate's older dependency manifest with the preserved newer controller. Preserve source and manifests; keep command and movement comparisons separate. Ordinary desktop input remains unresolved at supported activation; engine-action calibration does not prove it. This setup starts none of that work.

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
