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

Two private static cargo-action fixtures compiled across all five configurations and ran. Both retained 1,800 supplies in native storage and zero in the truck; neither started loading. They establish setup boundaries, not transfer or delivery:

- `native-cargo-action-v1`, pack `50587A63...`, failed its 90-second eligibility bound while Game Master remained open. Its [review](../.cache/client/runs/native-cargo-action-v1/independent-cargo-review.md) retains nine runtime errors and the declared 150-second forced close; natural cleanup was not observed.
- `native-cargo-action-v2`, pack `6801D9667AE41105C13304DF64DBEC87E4A505B82942E0797B157CFA4601FB66`, used supported native editor closure. Later guards and a live player-view capture confirmed editor/menu closed and `CanInteract == true`. The remaining action-eligibility conjunction still failed: native load/unload and cancellation remain unexercised. Its [review](../.cache/client/runs/native-cargo-action-v2/independent-cargo-review.md) retains nine runtime errors, zero recorded post-result errors and unobserved natural shutdown. Exact-prefix snapshot: +223 ms, SHA `9D32F83E91DD7254ED8F1A301D5F6F97FE6E24697472DDB556C88CEF37DA765F`. The client ended at its declared 150-second timer; watcher/recorder exits and fresh process/window checks were verified. Private video: 184.2 s, SHA `4384D735CBE5FF352542908283F069C4A130364F069200D5ADE55316B0C4AAE0`.

Compiled private source: `.cache/native-cargo-action-v2/source/ConvoyFollower`, 572 frozen files, source-manifest SHA `26D124C646CABA40F920313877A0DCC58E02735A2608B06756D98682FA34C9F6`. Probe SHA: `A603298E...74FC0F`. It is not canonical and changes no convoy driving. The source-only v3 observation candidate is ready at `.cache/native-cargo-action-v3/overlay/`, source SHA `11BC68E668A98D84C195C5A3098DAD9D3BBF6478486194BFBB49FBF3B5FB2C48`, manifest `1F42B29E...F6F1E097`. It separates duration/per-frame/show/perform predicates while preserving their order and conditions. It is uncompiled and unrun; root should copy the v2 private source to a new v3 copy, overlay this one file, compile/package and perform one bounded comparison. Identify the exact native rejection before changing fixture settings; do not bypass eligibility or set supply counts. A separate cache-only panel distance label remains uncompiled/unrun.

## Preserved failed controls

| Closed comparison | Result and useful boundary |
| --- | --- |
| [Trip v2, 25 km/h mixed pair](../.cache/client/runs/convoy-trip-two-v2/original-tail-independent.md) | Links 76.0872 / 63.9199 m fail spacing; guide → real predecessor → owned Wait arrival succeeds; 180.313 s zero drift |
| [Direct pair with head rear pacing](../.cache/client/runs/original-direct-rear-two-v1/paced-independent-result.md) | Links 83.8187 / 86.7728 m fail spacing; 180.346 s zero drift; simpler direct following is not a spacing fix |

Earlier route, native-request, explicit-Hold, Resume and input failures remain in the [validation history](convoy-validation-history-2026-09-26.md) and exact run directories. Do not attribute improvements to one change when timing, pace or multiple mechanisms differed.

## What is still unproved

- **Ordinary input:** supported desktop activation/click/key delivery remains unresolved, including vanilla controls. Capture and scripted engine-action calibration are not keyboard/panel proof. Elevated OSK accessibility exposed no supported Close action; its causal role remains unproved.
- **Cargo:** no completed native load/unload or delivered supply quantity is established. Use the [native supply test plan](convoy-native-supply-test-plan.md): real action dispatch, eligible player/context, before/after container counts and conservation; no count setters or transfer fallback.
- **Complete player workflow:** the [Supply Day scene](convoy-supply-showcase.md) is prepared, not a completed recruit/load/travel/unload/Resume demonstration.
- **Wider driving:** repeatable ordinary one-, two- and three-follower trips, turns, longer sessions, genuine moving dirt/grass contact and varied stops remain required. A mapped road or scene survey does not prove unpaved driving.
- **Interruptions and roles:** selected-member Hold with downstream waiting, rechain, predecessor/driver loss, possession, blocked recovery, unload/return and regroup need relevant ordinary-driver evidence. The private deferred-timeout recovery still requires StandDown/reassignment.
- **Multiplayer and release:** authority/ownership, joining and reconnecting players, USSR behavior, configuration, performance, distribution and a finished demonstration remain open. A first supply trip is an intermediate milestone.
- **Errors:** physical successes retain all runtime and shutdown failures. Matching vanilla signatures establishes independent occurrence, not harmlessness, ownership attribution or a waiver.

## Next usable-trip actions

1. Preserve the demonstrated ordinary-driver baseline; address concrete supply-trip blockers before broadening driving experiments.
2. Verify native cargo loading and unloading with conserved measured quantities. Label an autonomous native-action fixture as transfer proof only.
3. Resolve ordinary input through a supported, discriminating control; do not repeat unchanged key/click matrices or treat capture as input success.
4. Complete and record the ordinary Supply Day workflow with retained assignments, useful status and one spokesperson. Extend to two/three followers and realistic interruptions.
5. Continue the full roadmap: verified unpaved routes, regroup/return, role changes, Soviet and multiplayer coverage, performance/configuration and release polish.

## Evidence discipline and navigation

Keep the existing **60 m whole-run per-link peak**, exact original identities/predecessors, continuous powered travel and **180 s / 2 m** paced-hold gates. Explicit Hold requires **30 s** and Resume a fresh continuous **20 m / three powered intervals**. Command acceptance, projected route station and telemetry requests are not physical completion.

Preserve the complete console, terminal-prefix snapshot, frozen source/deployment hashes, physical and error verdicts, closure evidence and visual-review limits. Natural exit and a completed video do not erase errors. Raw full-desktop videos remain private.

- [Focused courses](convoy-feature-courses.md) · [Navigation/release plan](convoy-navigation-release-plan.md) · [Edge cases](convoy-edge-case-matrix.md)
- [Operations playbook](operations-playbook.md) · [Harness handoff](harness-handoff.md) · [Full historical validation](convoy-validation-history-2026-09-26.md)

Historical documents preserve dated observations. This checkpoint owns the latest result and next boundary; the full brief owns product scope, and the latest user instruction owns execution authority.
