# Current validation checkpoint

Updated September 26, 2026. **The mod is not release ready.** This page separates completed evidence from work still in progress.

**Execution: active development under the saved goal and latest user direction.** The earlier documentation-only setup turn is complete. Prioritize the usable supply trip and demonstration over perfecting isolated details. The latest user instruction controls scope; historical setup notes are not current stop instructions.

## Product goal and source checkpoint

Deliver a usable recruit/assign → load → follow → stop/Hold → unload → Resume trip and demonstration, then complete the broader release scope. Prioritize failures that interrupt that trip over additional diagnostic machinery.

- Public ordinary-driver checkpoint: **`ed127af`**. The exact tested integration is canonical: twelve source files and the generated database. Preserve it and the frozen comparisons; older brief revisions are not reset targets.
- Independent harness checkpoint: `eb42722`, including verified native Game Master play-mode, cargo setup and seated/idling delivery lessons, plus the explicitly unrun authenticated-join comparison. Generic tooling belongs there; addon scripts, worlds and convoy evidence belong here.
- The [full development brief](convoy-development-brief.md) remains unchanged. The [adaptive roadmap](convoy-development-reset.md) and [roadmap](roadmap.md) retain the complete product scope.
- Keep the actual predecessor chain, original assignments, one radio spokesperson and five-follower cap excluding the player's lead vehicle.
- Authored-source promotion is already complete. The [source checkpoint](convoy-authored-source-checkpoint.md) records that historical boundary; do not repeat export work without a concrete regression.

## Latest completed physical evidence

| Comparison | Physical result | Strict full-run result |
| --- | --- | --- |
| Cooperative 20 km/h lead, two followers | **PASS**: links 46.19 / 56.887887 m; 180.313 s zero measured drift | **FAIL**: 9 runtime / 0 post-result / 64 shutdown errors |
| Ordinary prefab, one follower, real server Hold/Resume | **PASS**: peak 55.4143 m; baseline 180.363 s; Hold 30.0659 s; continuous Resume 23.861953 m / 3 powered intervals | **FAIL**: 9 runtime / 0 post-result / 64 shutdown errors |
| Ordinary prefabs, two followers, same package | **PASS**: links 46.3392 / 55.5588 m; all three vehicles held 180.413 s with zero measured drift | **FAIL**: 9 runtime / 0 post-result / 62 shutdown errors |
| Ordinary one-follower native supply trip | **PASS**: load 100, carry to distinct destination, Hold/unload 100, Resume 22.9565 m; peak link 45.8258 m | **FAIL**: 9 runtime / 0 post-result / 59 shutdown errors |

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

### Complete automated supply-trip milestone

`loaded-supply-trip-v1` passed native load, ordinary follower travel, sustained arrival, real server Hold, unloading to a **distinct destination**, and powered Resume. Original source/truck/destination quantities were **1,800/0/0 → 1,700/100/0 → 1,700/0/100**, conserved through all 400 cargo records and the post-result period. The destination transfer occurred 326.469 m from the source. The same driver, truck and predecessor survived the entire sequence.

- Arrival held for **180.413 s**, then explicit Hold for **40.0991 s**, both with zero measured drift. Resume retained its fresh sequence 3 for **22.956498 m / nine powered intervals**. Whole-run peak gap was **45.8258 m** against 60 m.
- Pack `6361D179EA75890F0D4C2BFF1A11A53A98A4BD64622CBAFDE037257740353D50`; all five configurations passed. Independent review verified 577 frozen source hashes and all three deployment files.
- [Independent review](../.cache/client/runs/loaded-supply-trip-v1/independent-loaded-trip-review.md), SHA `B648D67D20014FB1DFB19CB4695AEF6C68C6109C26117794B649BFDFA6A26785`; snapshot was an exact prefix captured +200 ms. Natural closure, watcher/recorder completion and fresh process/window checks are verified. Nine gameplay / zero post-result / 59 shutdown errors remain.
- Raw recording: 453.6 s, SHA `0A443BD991461457154C9DB8AA4702A569371E6BC96FED0297563E341CF38221`. The 146-second [highlights](../.cache/test-videos/loaded-supply-trip-v1-highlights.mp4) and 390-second [gameplay crop](../.cache/test-videos/loaded-supply-trip-v1-gameplay.mp4) are silent. Three live views and sampled recorded frames were inspected; this is not full playback.
- The four exact tested regression resources are now canonical. They use native actions, with explicit test-only owner positioning and an AI lead. They do not automate ordinary worlds or prove normal keyboard/menu input. See the [supply-trip milestone](convoy-supply-trip-milestone.md).

The combined source, clearer **Resume after pull-off** wording and three-follower observer/world extension passed five-configuration validation and packaging in `.cache/workbench-runs/convoy-supply-integration-v1`, pack **`13B39DBF641FB57370962A05C2D13D9D5B52B11DE97FCD66372C627372D465A3`**. No follower controller or physical threshold changed in this integration. A separate panel distance-label candidate remains uncompiled/unrun.

### Three followers: useful travel, failed arrival

`ordinary-three-arrival-v1` ran the same ordinary controller with a third recruited driver and independent per-unit route observations. All three retained their actual predecessors and achieved over 300 m of continuous powered movement; both tails joined their own predecessor's recorded route. Whole-run peak gap was **57.3073 m**. However, Unit Two's native movement request returned **UNREACHABLE** during arrival, and its movement lease entered permanent failure. The observer correctly failed at 113.05 seconds; **zero formal arrival-observation samples** were completed. Later stopping by other trucks does not repair that verdict.

The [independent review](../.cache/client/runs/ordinary-three-arrival-v1/independent-three-review.md) checked the frozen 580-file source, three deployment files, 212 packed text resources and an exact-prefix snapshot captured +61 ms. Full evidence remains in that run directory, including **9 runtime / 0 post-result / 64 shutdown errors**. Natural client closure, watcher and recorder exit and fresh process/window checks were confirmed. The raw silent recording is 215.733333 seconds, SHA `B8134A582C162516988484CEADEF02520C82215E650DD8A5F6A544D6E06DB116`. One live travel view and a recorded arrival frame were inspected. This is a product-relevant recovery/arrival fault to fix, not a three-follower pass.

Source investigation confirmed installed `EMoveError.UNREACHABLE` on the exact admitted vehicle handler, rather than an identity/ownership rejection. Unit Two was genuinely joined near the end of its recorded guide; Unit One was still repositioning and selected its arrival Wait 14.634 seconds later. This motivated the bounded recovery tested below; the earlier failed run remains unchanged.

### Bounded arrival recovery: demonstrated improvement, spacing still fails

`ordinary-three-arrival-recovery-v1` used the same world, 20 km/h lead, settings and physical gates with three controller/lease changes. Unit Two's exact admitted native request failed with UNREACHABLE at16:08:08.772. The failed/revoked lease remains in the log. The controller retired only that owned request, selected a separate seated recovery Wait, and issued one fresh real-predecessor 10m approach after Unit One selected its actual arrival Wait. Unit Two then passed normal capture at16:08:19.939. All three captured arrival; all four trucks held **180.543s /180 samples with zero measured drift**.

The full physical/strict result remains **FAIL**: Unit Three had already exceeded spacing before recovery, peaking at **67.7483m**. Unit One/Two peaks were55.1667/59.4984m. Their original exact moving segments were323.213/314.236/315.450m with33/34/31 powered intervals; identities and genuine predecessor joins were retained. The arrival improvement does not erase the earlier spacing failure.

- All five configurations and packaging passed; data SHA-256 **`39ACB01A7B39527E357BF13960756CD2C13665DF17B930D74BC2829A4A943CE7`**. The exact three scripts and generated database are canonical; promotion record `.cache/arrival-recovery-v1/promotion.json` preserves prior files.
- [Independent review](../.cache/client/runs/ordinary-three-arrival-recovery-v1/independent-three-review.md), SHA`67E226C151C55EB4053CAFFBE347D570401B1F17216A84945B83C41AC14A34CD`, verifies580 sources,212 packed text resources,3 deployment files and exact-prefix snapshot +178ms. **9 runtime /0 post-result /64 shutdown errors** remain. Natural client exit, watcher/recorder completion and fresh process/window closure are recorded.
- [140-second game-view crop](../.cache/test-videos/ordinary-three-arrival-recovery-v1-gameplay.mp4) covers raw65–205s, SHA`1D0E28A3B42316CF1586AAE56D41D7CF6A08E125A34998E61B374307AC689E4F`. The private raw recording is405.533333s, SHA`5785B83AD66B13CDD67A28F7DD90E7CDB50B47FA65724157F31C26F28646594A`. Silent; three live captures and one recorded arrival frame inspected, not full playback.
- [Surface review](../.cache/client/runs/ordinary-three-arrival-recovery-v1/surface-review.md) establishes actual moving dirt contact for lead and all three followers:10/10/13/15 qualifying samples, each with six contacting wheels. Unit Two briefly contacted grass while moving. This is a mapped mixed road, not continuous grass or off-network proof.
- Recovery timeout, command interruption and possession branches are source-reviewed but not positively exercised. Normal player input and multiplayer remain separate requirements.

The next private comparison enables the existing rear-pacing mechanism on the middle follower: it was accelerating while its successor was already at full throttle on the climb. Only the ordinary controller policy and its observer's exact expected-policy attribution change; world, lead speed, pacing algorithm and physical gates remain fixed. Pack`AC5D5AC7A8786C7E6AC46B749AB73C61D65FCBE283CE2F8D106D035F33948BF9` passed all five configurations; live `ordinary-three-middle-pacing-v1` is pending final review. Do not claim a spacing improvement before that result.

## Preserved failed controls

| Closed comparison | Result and useful boundary |
| --- | --- |
| [Trip v2, 25 km/h mixed pair](../.cache/client/runs/convoy-trip-two-v2/original-tail-independent.md) | Links 76.0872 / 63.9199 m fail spacing; guide → real predecessor → owned Wait arrival succeeds; 180.313 s zero drift |
| [Direct pair with head rear pacing](../.cache/client/runs/original-direct-rear-two-v1/paced-independent-result.md) | Links 83.8187 / 86.7728 m fail spacing; 180.346 s zero drift; simpler direct following is not a spacing fix |

Earlier route, native-request, explicit-Hold, Resume and input failures remain in the [validation history](convoy-validation-history-2026-09-26.md) and exact run directories. Do not attribute improvements to one change when timing, pace or multiple mechanisms differed.

## What is still unproved

- **Ordinary input:** supported desktop activation/click/key delivery remains unresolved, including vanilla controls. Capture and scripted engine-action calibration are not keyboard/panel proof. Elevated OSK accessibility exposed no supported Close action; its causal role remains unproved.
- **Cargo:** native load, transport, distinct-destination unload and Resume are verified for one ordinary follower in the automated fixture. Multi-truck delivery and the ordinary cargo menu/input path remain unproved. Retain native eligibility, exact container counts and conservation; no count setters or transfer fallback.
- **Complete player workflow:** the [Supply Day scene](convoy-supply-showcase.md) is prepared, not a completed recruit/load/travel/unload/Resume demonstration.
- **Wider driving:** repeatable ordinary one-, two- and three-follower trips, turns, longer sessions, sustained grass/off-network travel and varied stops remain required. Moving dirt contact is now measured for all participants on the mapped mixed course; a scene survey alone remains insufficient.
- **Interruptions and roles:** selected-member Hold with downstream waiting, rechain, predecessor/driver loss, possession, blocked recovery, unload/return and regroup need relevant ordinary-driver evidence. The private deferred-timeout recovery still requires StandDown/reassignment.
- **Multiplayer and release:** authority/ownership, joining and reconnecting players, USSR behavior, configuration, performance, distribution and a finished demonstration remain open. A first supply trip is an intermediate milestone.
- **Errors:** physical successes retain all runtime and shutdown failures. Matching vanilla signatures establishes independent occurrence, not harmlessness, ownership attribution or a waiver.

## Next usable-trip actions

1. Preserve the demonstrated ordinary-driver baseline; address concrete supply-trip blockers before broadening driving experiments.
2. Preserve the demonstrated bounded arrival recovery and successful one-follower delivery. Complete the middle-link rear-pacing comparison for the remaining three-follower spacing failure; inspect all links and actual pacing activation, not only the tail gap. Exercise interruption of recovery when a concrete command fixture is ready.
3. Resolve ordinary input through a supported, discriminating control; do not repeat unchanged key/click matrices or treat capture as input success.
4. Complete and record the ordinary Supply Day workflow with retained assignments, useful status and one spokesperson. Extend to two/three followers and realistic interruptions.
5. Continue the full roadmap: verified unpaved routes, regroup/return, role changes, Soviet and multiplayer coverage, performance/configuration and release polish.

## Evidence discipline and navigation

Keep the existing **60 m whole-run per-link peak**, exact original identities/predecessors, continuous powered travel and **180 s / 2 m** paced-hold gates. Explicit Hold requires **30 s** and Resume a fresh continuous **20 m / three powered intervals**. Command acceptance, projected route station and telemetry requests are not physical completion.

Preserve the complete console, terminal-prefix snapshot, frozen source/deployment hashes, physical and error verdicts, closure evidence and visual-review limits. Natural exit and a completed video do not erase errors. Raw full-desktop videos remain private.

- [Focused courses](convoy-feature-courses.md) · [Navigation/release plan](convoy-navigation-release-plan.md) · [Edge cases](convoy-edge-case-matrix.md)
- [Operations playbook](operations-playbook.md) · [Harness handoff](harness-handoff.md) · [Full historical validation](convoy-validation-history-2026-09-26.md)

Historical documents preserve dated observations. This checkpoint owns the latest result and next boundary; the full brief owns product scope, and the latest user instruction owns execution authority.
