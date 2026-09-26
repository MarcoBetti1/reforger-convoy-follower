# Current validation checkpoint

Updated September 26, 2026. **The mod is not release ready.** This page separates completed evidence from work still in progress.

**Execution: active development under the saved goal and latest user direction.** The earlier documentation-only setup turn is complete. Prioritize the usable supply trip and demonstration over perfecting isolated details. The latest user instruction controls scope; historical setup notes are not current stop instructions.

## Product goal and source checkpoint

Deliver a usable recruit/assign → load → follow → stop/Hold → unload → Resume trip and demonstration, then complete the broader release scope. Prioritize failures that interrupt that trip over additional diagnostic machinery.

- Public ordinary-driver checkpoint: **`ed127af`**. The exact tested integration is canonical: twelve source files and the generated database. Preserve it and the frozen comparisons; older brief revisions are not reset targets.
- Independent harness checkpoint: `c18fa8a`, including the verified native Game Master play-mode setup lesson. Generic tooling belongs there; addon scripts, worlds and convoy evidence belong here.
- The [full development brief](convoy-development-brief.md) remains unchanged. The [adaptive roadmap](convoy-development-reset.md) and [roadmap](roadmap.md) retain the complete product scope.
- Keep the actual predecessor chain, original assignments, one radio spokesperson and five-follower cap excluding the player's lead vehicle.
- Authored-source promotion is already complete. The [source checkpoint](convoy-authored-source-checkpoint.md) records that historical boundary; do not repeat export work without a concrete regression.

## Latest completed physical evidence

| Comparison | Physical result | Strict full-run result |
| --- | --- | --- |
| Cooperative 20 km/h lead, two followers | **PASS**: links 46.19 / 56.887887 m; 180.313 s zero measured drift | **FAIL**: 9 runtime / 0 post-result / 64 shutdown errors |
| Ordinary prefab, one follower, real server Hold/Resume | **PASS**: peak 55.4143 m; baseline 180.363 s; Hold 30.0659 s; continuous Resume 23.861953 m / 3 powered intervals | **FAIL**: 9 runtime / 0 post-result / 64 shutdown errors |
| Ordinary prefabs, two followers, same package | **PASS**: links 46.3392 / 55.5588 m; all three vehicles held 180.413 s with zero measured drift | **FAIL**: 9 runtime / 0 post-result / 62 shutdown errors |

### Cooperative two-follower milestone

Run: [cooperative20-two-v1 final report](../.cache/client/runs/cooperative20-two-v1/original-tail-independent.md).

The direct head and recorded-predecessor tail retained their exact moving activities for 323.237387 m / 35 powered intervals and 313.064334 m / 38 intervals. Both selected their own arrival Wait; all three trucks held for 180.313 seconds with zero measured drift.

The declared lead request changed from 25 to **20 km/h**; follower code and physical thresholds did not change. This is a useful cooperative condition on the tested route, not a general maximum human speed or a repair of the failed 25 km/h controls.

- Five script configurations and packaging passed in `.cache/workbench-runs/convoy-cooperative20-v1/`.
- Data SHA-256: `48D3F65CE5D36E00108249BC402B97B1521088F2102185A27348DDDD84D88B90`.
- Final evidence verifies 560 frozen sources, 197 packed text resources, 38 comparison resources and all three deployment files.
- Terminal snapshot is an exact log prefix captured +34 ms; natural closure and recording completion are verified.
- A game-view crop was shown. Visual inspection was sampled, not full playback; the course is mapped mixed road, not wholly unpaved/off-network proof.

### Ordinary one-follower milestone

Run: [ordinary-one-resume-v1 final report](../.cache/client/runs/ordinary-one-resume-v1/original-resume-independent-result.md).

Ordinary US prefab/controller and committed head role were verified. Baseline driving and the 180.363-second selected-Wait observation passed. Real server Hold then completed 30.0659 seconds with zero drift. Resume retained the exact fresh sequence 3 and produced 23.861953 m / three qualifying powered intervals; original identities survived.

- Ordinary candidate data SHA-256: `7ED4448FC670B14F4557DC68F202E8A14FE0A8EC31EA6227049506057B736464`.
- All five script configurations and packaging passed. Final verification covers 568 frozen sources, 203 packed text resources, 56 pinned resources and three deployment files.
- Report SHA-256: `B02FA32216ACE25A0C65AB945635D65FE65C8ED80FD0EE16C0A7960F25753FD9`.
- Exact-prefix snapshot captured +69 ms; natural client closure, watcher and recorder completion are verified.
- The lead requests 20 km/h for both legs; the spacing-only diagnostic bypass is disabled.
- Fixture recruitment and server commands exercise ordinary driver code, but do not prove ordinary keyboard/mouse interaction, human driving, cargo delivery or multiplayer.
- Three live views covered travel, parked arrival and renewed travel. This is sampled visual evidence, not full playback.

## Ordinary two-follower milestone and integrated baseline

Run: [ordinary-two-arrival-v1 final report](../.cache/client/runs/ordinary-two-arrival-v1/original-tail-independent.md).

The same package used ordinary groups and production driver prefabs without character-slot overrides. Unit One followed the actual lead, and Unit Two genuinely joined its immediate predecessor's recorded route. Their exact moving activities produced 322.232855 m / 35 powered intervals and 310.183320 m / 30 intervals. Both captured their own arrival Wait; all three vehicles held for 180.413 seconds with zero measured drift. Peak links were 46.3392 / 55.5588 m against the unchanged 60 m gate. One brief Unit One reverse episode remains (0.4007 s, about 0.2691 m); do not advertise perfectly smooth arrival.

- Final report SHA-256: `529A06619C6CF5F3CF1466CE3E1AF418D3A738324B9DCF1D9A47B9BA3B1E1F4E`.
- Snapshot is an exact prefix captured +161 ms. Natural client closure, watcher and recorder exit and fresh process/window checks are verified.
- Full private recording: 515.866667 s, SHA-256 `96356947751563573BB612E03A27D176381E3356E4981FF562AA5D169EF037A7`. It includes post-game desktop footage; only sampled live travel was inspected.
- Verification covers the same 568 sources, 203 packed text resources, 56 pins and three deployment files. Nine runtime / zero post-result / 62 shutdown errors remain; physical PASS does not waive strict failure.
- This case does not include a two-driver explicit Hold/Resume episode. The one-driver command result cannot substitute for it.

The exact tested controller and US/USSR prefab wiring, matching observers and two worlds are now canonical. The promotion record is `.cache/ordinary-gameplay-v1/promotion.json`; prior files are preserved there. Only the US one- and two-follower cases have physical evidence. Soviet behavior, additional roles, three followers and ordinary UI input are unproved.

## Current product boundary

Preserve this working movement/arrival baseline and progress to real native supply actions and the [ordinary Supply Day scene](convoy-supply-showcase.md). The ordinary two-driver gameplay crop, `.cache/test-videos/ordinary-two-arrival-v1-gameplay.mp4`, was shown after inspecting a sampled arrival frame; it is driving/arrival footage, not a complete supply demonstration.

### Native loading and unloading now verified

`native-cargo-action-v6` passed the static native-action transfer/cancellation test on pack **`CC39384DD7C1A9DFFE64EC180A781FD03B0C5AFBBC313F8736F0A31E10A7EE4C`**. Original storage/truck counts moved **1,800/0 → 1,700/100 → 1,800/0**. Both native actions and cancellations ran; counts remained stable for 3.000 and 3.0154 seconds afterward. Exact original containers and single-contributor queues were retained. Unload discovery needed 33.1 ms after its first eligibility check. No runtime supply setters, vehicle controls or eligibility bypass were used.

- All five configurations and packaging passed in `.cache/workbench-runs/convoy-native-cargo-action-v6`.
- The four exact regression resources are now canonical: `CF_NativeCargoActionProbeComponent.c` and `Worlds/Tests/ConvoyFollower_Arland_StaticCargoAction.ent` with its metadata/layer. They run only in the opted-in static world. Promotion record: `.cache/native-cargo-action-v6/promotion.json`.
- Frozen 572-file source manifest: `3FC1CBE539694E133E2EED295A14AC79AEA4243CEAD98C00ED5179E459F696E1`. Snapshot: +209 ms, `E3AC5558E7D32C492789C26474A85BACF4A3FCF75CD52528B92409B5A8CB4094`.
- [Independent review](../.cache/client/runs/native-cargo-action-v6/independent-cargo-review.md) and closure record retain nine runtime errors and the 150-second client timer; natural shutdown is unobserved. Watcher/recorder completion and fresh process/window closure were verified. Video: 240 s, SHA `EA06946303E53AE3D3EF4B19C54C92453413F4A5B423AF82BF4FD955236775B2`; only one live post-result view was inspected.
- This proves static native-action load/unload at the same storage. It is **not transport to a destination, ordinary input or multiplayer evidence**.

Earlier cargo attempts remain preserved in `.cache/client/runs/native-cargo-action-v1` through `v5`: v1 exposed an open Game Master editor; v2 closed it; v3 isolated disabled scenario supplies; v4 reached eligibility but failed a stationary check before dispatch; v5 loaded/canceled 100 but rejected unloading before its spatial subscription had populated. Each keeps its full log and review. V4's exact failing physical term was not logged; settling remains an inference. None of these earlier attempts is a complete round-trip pass.

The ordinary Supply Day scene now explicitly enables native SUPPLIES once and authors source/destination quantities on their real child containers. This pair passed five-configuration validation and packaging (`BAFCCAFB46F0FF8D5D805DD9CDCD206A95EC4761E0D0CA3E311C3CE679D21F7F`). The combined canonical source passed all five configurations and packaging in `.cache/workbench-runs/convoy-supply-regression-integrated-v1`, pack `7AC9BDCCD9CAADBE4DE90825E2A1B647B6748DA793B91265AF9D554DFCD6073C`. This combined package has not had an additional live run; its exact cargo source matches the measured v6 test. No automatic transfer, recruitment or driver assistance was added to the ordinary scene.

Next combine native load, the proven ordinary one-follower route, Hold, unloading to a **distinct** destination and Resume. The private candidate is being authored under `.cache/loaded-supply-trip-candidate`; it is not compiled or run. Explicit scripted owner staging is test setup, not normal player-input proof. A separate panel distance-label candidate remains uncompiled/unrun.

## Preserved failed controls

| Closed comparison | Result and useful boundary |
| --- | --- |
| [Trip v2, 25 km/h mixed pair](../.cache/client/runs/convoy-trip-two-v2/original-tail-independent.md) | Links 76.0872 / 63.9199 m fail spacing; guide → real predecessor → owned Wait arrival succeeds; 180.313 s zero drift |
| [Direct pair with head rear pacing](../.cache/client/runs/original-direct-rear-two-v1/paced-independent-result.md) | Links 83.8187 / 86.7728 m fail spacing; 180.346 s zero drift; simpler direct following is not a spacing fix |

Earlier route, native-request, explicit-Hold, Resume and input failures remain in the [validation history](convoy-validation-history-2026-09-26.md) and exact run directories. Do not attribute improvements to one change when timing, pace or multiple mechanisms differed.

## What is still unproved

- **Ordinary input:** supported desktop activation/click/key delivery remains unresolved, including vanilla controls. Capture and scripted engine-action calibration are not keyboard/panel proof. Elevated OSK accessibility exposed no supported Close action; its causal role remains unproved.
- **Cargo:** static native load/unload is verified; transport and delivery to a distinct destination remain unproved. Use the [native supply test plan](convoy-native-supply-test-plan.md): real action dispatch, eligible player/context, before/after container counts and conservation; no count setters or transfer fallback.
- **Complete player workflow:** the [Supply Day scene](convoy-supply-showcase.md) is prepared, not a completed recruit/load/travel/unload/Resume demonstration.
- **Wider driving:** repeatable ordinary one-, two- and three-follower trips, turns, longer sessions, genuine moving dirt/grass contact and varied stops remain required. A mapped road or scene survey does not prove unpaved driving.
- **Interruptions and roles:** selected-member Hold with downstream waiting, rechain, predecessor/driver loss, possession, blocked recovery, unload/return and regroup need relevant ordinary-driver evidence. The private deferred-timeout recovery still requires StandDown/reassignment.
- **Multiplayer and release:** authority/ownership, joining and reconnecting players, USSR behavior, configuration, performance, distribution and a finished demonstration remain open. A first supply trip is an intermediate milestone.
- **Errors:** physical successes retain all runtime and shutdown failures. Matching vanilla signatures establishes independent occurrence, not harmlessness, ownership attribution or a waiver.

## Next usable-trip actions

1. Preserve the demonstrated ordinary-driver baseline; address concrete supply-trip blockers before broadening driving experiments.
2. Reuse the verified native cargo actions in the ordinary-driver trip: load, physical travel, Hold, unload to a distinct destination, then Resume with retained assignments.
3. Resolve ordinary input through a supported, discriminating control; do not repeat unchanged key/click matrices or treat capture as input success.
4. Complete and record the ordinary Supply Day workflow with retained assignments, useful status and one spokesperson. Extend to two/three followers and realistic interruptions.
5. Continue the full roadmap: verified unpaved routes, regroup/return, role changes, Soviet and multiplayer coverage, performance/configuration and release polish.

## Evidence discipline and navigation

Keep the existing **60 m whole-run per-link peak**, exact original identities/predecessors, continuous powered travel and **180 s / 2 m** paced-hold gates. Explicit Hold requires **30 s** and Resume a fresh continuous **20 m / three powered intervals**. Command acceptance, projected route station and telemetry requests are not physical completion.

Preserve the complete console, terminal-prefix snapshot, frozen source/deployment hashes, physical and error verdicts, closure evidence and visual-review limits. Natural exit and a completed video do not erase errors. Raw full-desktop videos remain private.

- [Focused courses](convoy-feature-courses.md) · [Navigation/release plan](convoy-navigation-release-plan.md) · [Edge cases](convoy-edge-case-matrix.md)
- [Operations playbook](operations-playbook.md) · [Harness handoff](harness-handoff.md) · [Full historical validation](convoy-validation-history-2026-09-26.md)

Historical documents preserve dated observations. This checkpoint owns the latest result and next boundary; the full brief owns product scope, and the latest user instruction owns execution authority.
