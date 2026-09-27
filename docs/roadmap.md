# Roadmap

**Current execution status — September 26, 2026: development resumed.** The documentation-only setup turn is complete. The user's subsequent active-goal continuation authorizes implementation and live testing under the full brief. Preserve earlier development and evidence.

## Current product direction — September 26, 2026

The full [Convoy Follower development brief](convoy-development-brief.md) supersedes the bounded overnight direction and the original harness plan below. Use [the adaptive product roadmap](convoy-development-reset.md) for milestone sequencing, [the current validation record](convoy-validation-current.md) for observed results, and [the navigation plan](convoy-navigation-release-plan.md) for driving experiments and release gates.

The complete brief is saved unchanged and reconciled with the existing roadmap. Its older restart commits describe historical context; preserve source changes and evidence beyond that checkpoint. The latest emphasis on a useful, demonstrable mod determines the resumed development priority.

## Current development boundary

The tested ordinary controller is integrated into US/USSR driver prefabs. US one- and two-follower travel and sustained arrival passed at a declared 20 km/h lead pace. Preserve the completed one-follower load/transport/Hold/delivery of 100 supplies/powered Resume baseline. The two-loaded-truck case now also delivered **200 supplies** to a distinct destination, retained both assignments, completed **180.313 s arrival** and **48.1325 s explicit Hold** with zero measured drift. Its overall result remains **FAIL**: Unit Two peaked at **81.625 m** against the 60 m spacing gate, then production Resume failed with `measured_step_spans_multiple_vertices` before fresh movement. Nine gameplay and 64 shutdown errors remain. See [current validation](convoy-validation-current.md) for exact results and failed controls.

The two-truck regression resources are canonical. Later private straight-course candidates completed sustained arrival and real Hold. The [V3 comparison](experiments/loaded-straight-trip-v3.md) delivered 200 supplies, completed the held seat handoff, accepted owner-passenger Resume, and moved all three vehicles roughly 38–40 m. It still failed the 60 m spacing gate and the short restart leg did not establish each required fresh powered segment. Preserve this useful progress without calling it a full pass. [Current validation](convoy-validation-current.md) owns exact outcomes. Normal player input, a qualified complete two-truck restart, Soviet behavior and multiplayer remain unproved.

Owner-passenger Resume in the same established lead vehicle and the ordinary Supply Day source placement are now canonical and packaged. Private driving candidates remain separate. Preserve V4's 200-supply delivery and held onward attempt. The [V5 comparison](experiments/loaded-straight-trip-v5.md) passed 112 native geometry cases but failed initial travel: native backward movement near x1405 preceded route-corridor loss, with spacing already failed. Investigate that native path boundary and an alternate representative course while retaining the failed course and unchanged gates; do not infer that the new projection helper caused the reversal. The independent selected Reboard candidate is compiled/packaged and running, with no completed result yet; it must preserve assignment and return to Hold. Representative driving regressions and ordinary-control validation remain required.

### Immediate playable milestone

The latest user direction is to zoom out and deliver a working mod that can be shown in use. Concentrate the next iteration on **recruit/assign → load → travel → stop/Hold → unload → Resume**, with the original trucks and membership preserved. Record that complete journey, state which interactions are automated, and carry demonstrated improvements into ordinary gameplay.

Prioritize a smaller fix when it blocks that journey. Use the established fixtures to diagnose it, then return to the full trip; avoid extending diagnostic machinery or adding optional polish without a specific product benefit. Preserve the harder junction and three-follower failures as regression requirements. A successful straight-road demonstration is progress toward those requirements, not a replacement for them.

The playable milestones remain: dependable movement/hold/restart; an actual supply trip through normal player controls; multiple followers and interruptions; assignment, individual commands, arrival/regroup/return; and feedback, multiplayer, configuration, performance and distribution. Keep one spokesperson and the five-follower cap, excluding the player lead. A first supply demonstration is a meaningful milestone within the full brief.

Generic tools and lessons belong in the independent [Reforger Agent Harness repository](https://github.com/MarcoBetti1/reforger-agent-harness); addon scripts, assets, worlds and convoy evidence belong here. Preserve the current integrated controller, supply regressions and frozen failed comparisons; [current validation](convoy-validation-current.md) owns the latest source checkpoints. Workshop publication remains separate.

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
