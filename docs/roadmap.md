# Roadmap

**Current execution status — September 26, 2026: development resumed.** The subsequent active-goal continuation (`updatedAt: 1790444332`) authorizes development and live validation under the full brief. Setup-only descriptions below record the completed documentation turn; they do not override this continuation. Preserve newer work and completed evidence.

## Current product direction — September 26, 2026

The full [Convoy Follower development brief](convoy-development-brief.md) supersedes the bounded overnight direction and the original harness plan below. Use [the adaptive product roadmap](convoy-development-reset.md) for milestone sequencing, [the current validation record](convoy-validation-current.md) for observed results, and [the navigation plan](convoy-navigation-release-plan.md) for driving experiments and release gates.

The complete brief is saved unchanged and reconciled with the existing roadmap. This setup leaves the saved goal untouched; work toward the milestones below awaits the user's update and resumption. Preserve source changes and evidence beyond the older pause checkpoint.

## Current development boundary

**Immediate product priority:** the user has asked to focus on a working convoy trip and demonstration rather than perfecting small details. Complete a usable recruit/load/follow/stop/unload/resume loop, address interruptions first, and document rough edges. The latest projected-station experiment passed its native helper cases and extended physical travel, but still stopped at another route guard; it is useful evidence, not a completed driving solution. Current validation records the exact result. Keep the full product direction while sequencing work around this playable milestone.

Preserve current mod checkpoint `39080da`, its authored-source predecessor `42f48b3`, independent harness checkpoint `642332a`, all earlier history, and newer local source, frozen packs and evidence. The entry-guide experiment already tested in `2BCA4FC7...` is now versioned in `39080da`; do not reset it. The historical commits quoted in the brief are reference points, not reset targets. The [current validation record](convoy-validation-current.md) owns exact results and hashes.

The common private package `46122D43...` has a finalized one-follower Hold/Resume physical PASS with strict runtime/shutdown failure. Direct-two and tail-two have finalized physical/strict failures; tail spacing failed before its genuine route join and later geometry rejection. The separately built clean authored package `378848C4...` has a finalized one-follower physical PASS / strict full-run FAIL with unchanged gates. Natural process closure and the 840-second private recording are verified; runtime/shutdown errors remain. These comparisons do not establish a production backend or release readiness. Preserve earlier failed controls and all exact package attribution.

Authored-source export revision two passed five-configuration validation and packaging in a separate copy. Its 533-source/180-text-resource verification and finalized Hold/Resume result are recorded in current validation. Canonical authored-source promotion is complete and versioned in `42f48b3`; normal production driver prefabs remain unchanged. Production adoption and distribution review remain separate gates. Keep copied comparators and raw evidence private. Ordinary desktop/panel input remains unresolved, and engine-action calibration does not prove it.

The later entry-guide comparison is already built and finalized: physical/strict FAIL on `2BCA4FC7...`, with a 73.8844 m tail peak over its recorded horizon, a genuine route join, then a separate geometry rejection before formal hold. Natural closure, recording and nine runtime / 62 shutdown errors are preserved. Earlier join is useful evidence, but the earlier terminal prevents treating its lower peak as a full-course improvement. Post-failure review also found misleading recovery messages without a newly created movement request.

Next comparisons after the completed resumption: compare corner projection, guide-aware convoy pacing, and truthful blocked-state handling as separate hypotheses, with one causal change per comparison. The nine-file projected-station candidate is now privately integrated, compiled and run: all 61 native cases pass, but the physical two-follower comparison fails spacing and a later first-vertex guard. See current validation for finalized evidence. Its larger station advances are a semantic decision, not an established tolerance fix; pacing must preserve the real predecessor and exact owned guide identity. Preserve draft artifacts and keep successful private command behavior, unchanged acceptance gates and the original chain. Clean-run closure and authored-source promotion are complete and should not be repeated. The setup turn is complete; current execution follows the active goal and latest product priority.

The playable milestones are dependable movement/hold/restart; a complete supply trip through ordinary player controls; multiple followers and interruptions; complete assignment, individual commands, arrival/regroup/return; and a polished, supported release candidate covering feedback, multiplayer, configuration, performance, and distribution. The first supply trip is an intermediate milestone. Preserve one radio spokesperson and the five-follower cap excluding the player's lead vehicle. Autonomous live verification, actual cargo transfer, human input restoration, multiplayer evidence, and a real-map demonstration remain required. There is no artificial overnight deadline, attempt limit or user-imposed credit budget; future execution follows the full brief and the user's subsequent goal instructions.

Generic tools and operating lessons belong in the independent [Reforger Agent Harness repository](https://github.com/MarcoBetti1/reforger-agent-harness); addon source, assets, worlds, convoy reporters, and gameplay evidence belong here. Raven research is closed. Workshop publication remains separate from development and release preparation.

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
