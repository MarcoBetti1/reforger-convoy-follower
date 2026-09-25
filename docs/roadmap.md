# Roadmap

## Completed baseline: repeatable operations

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

## Phase 1: Foundations

- Keep this repository as the control plane for the helper.
- Initialize Git and GitHub.
- Store design notes, prompts, and helper code here.
- Keep actual mod source in separate repos or separate folders with their own Git history.

## Phase 2: Read-only mod intelligence

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

## Phase 3: Safe editing loop

- Add guarded write tools for text files only.
- Require edits to stay inside configured roots.
- Require the agent to report exact file paths and a diff summary.
- Use one branch per requested mod change.

## Phase 4: Build and test automation

- Add wrappers for Workbench CLI or plugin-driven tasks.
- Add logs and failure summaries back into the agent context.
- Start with dry-run validation and packaging, not direct publishing.

## Phase 5: CI/CD

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

## Phase 6: First real project

- Create or clone a small learning mod.
- Use the agent to make one change at a time.
- Test locally after every change.
- Build a library of successful prompts and failure cases.
