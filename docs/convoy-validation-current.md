# Current validation checkpoint

Updated September 26, 2026. **The mod is not release ready.** This page separates completed evidence from work still in progress.

**Execution: active development under the saved goal and latest user direction.** The earlier documentation-only setup turn is complete. Prioritize the usable supply trip and demonstration over perfecting isolated details. The latest user instruction controls scope; historical setup notes are not current stop instructions.

**Latest boundary:** `straight-arrival-stop-resume-v2` is closed and failed. Native route geometry passed 100/100, but the physical run exceeded spacing and lost reverse-query context during the guide-to-arrival handoff. The working addon remains unchanged; [the original experimental patch](experiments/straight-arrival-v1.md) and [V2 increment](experiments/straight-arrival-v2.md) preserve these candidates. Next repair continuous actual-pose observation across the owned handoff, then complete the same stop/Hold/Resume and loaded-trip workflow. Investigate the earlier spacing fault separately; do not promote the failed run or restart completed geometry investigations.

## Product goal and source checkpoint

Deliver a usable recruit/assign → load → follow → stop/Hold → unload → Resume trip and demonstration, then complete the broader release scope. Prioritize failures that interrupt that trip over additional diagnostic machinery.

- Public controller checkpoint: **`e8e3b01`**, including the tested bounded arrival recovery. The ordinary-driver integration began at `ed127af`; the native supply-trip milestone is retained at `763aa1c`. Preserve these increments and the frozen comparisons; older brief revisions are not reset targets.
- Independent harness checkpoint: `fe68531`, including verified native Game Master play-mode, cargo setup, initial multi-vehicle settling, seated/idling delivery and Enforce compilation lessons, plus reusing proven start/pilot/goal geometry for focused tests. The authenticated-join comparison remains explicitly unrun. Generic tooling belongs there; addon scripts, worlds and convoy evidence belong here.
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

### Middle-link pacing: exercised, departure spacing still fails

`ordinary-three-middle-pacing-v1` enabled the existing rear-pacing policy on the middle follower and updated only the observer's expected-policy attribution. World, 20 km/h lead, pacing algorithm and physical gates remained fixed. All five configurations passed for pack **`AC5D5AC7A8786C7E6AC46B749AB73C61D65FCBE283CE2F8D106D035F33948BF9`**. This candidate remains private.

All three followers traveled 322.841 /313.750 /321.104 m with41/30/30 powered intervals, captured arrival normally and retained assignments. All four trucks held **180.493 seconds /180 samples with zero measured drift**. No arrival recovery was needed. Physical/strict result remains **FAIL** because Unit Three peaked at **63.7183 m**. The different peak from the earlier run is not a proven causal improvement.

The policy genuinely ran: Unit Two had49 eligible observations and35 reduced-cap observations; the last unit correctly had no successor. But admission came too late during initial departure: Unit Three genuinely joined its guide at16:19:25.492, Unit Two first admitted feedback at25.542 with a59.5121m rear gap, and the60m gate failed at25.711. The first logged reduced cap came at26.608. Earlier, the exact bound successor was already moving under power in its entry approach, which the existing pacing predicate excluded.

- [Independent review](../.cache/client/runs/ordinary-three-middle-pacing-v1/independent-three-review.md) verifies the unchanged physical gates and source/deployment provenance. Full log SHA`B709F406080637B0851F6DD9BE649BCD4BCC67BE43E34FB74E140CCBE6FDB6A4`; terminal snapshot SHA`03FC7C12919565B37464C5199FE6924D34D16A5BA626FA8FA6A58078F2585C2A`, exact prefix captured+21ms.
- Natural exit, watcher/recorder completion and fresh empty game/process inventory are in `closure-review.json`. **9 runtime /0 post-result /64 shutdown errors** remain.
- Private silent desktop recording:516.4s, SHA`037A70CA31D4A02A2E2002599E721E4A371CE86067CB44729DC05B626F9D0D88`. One live arrival view was inspected; no full-playback or ordinary-input claim.

### Earlier entry feedback: rejected candidate

`ordinary-three-entry-pacing-v1` added pacing-only admission for a healthy exact recorded entry approach, preserving genuine join for route/progress credit, the physical gates and the single cruise writer. All five configurations and packaging passed for **`EDC4B52E856E4786647B81E6B8C9C54D02A347BFCEB4C40F6F127B3D23E19334`**. The frozen580-file source differed from the prior middle-pacing candidate only in the trail controller. **Neither pacing candidate is promoted.**

The physical result worsened: link peaks **60.3095 /93.3601 /64.334m**, terminal failure at118.276s and **zero formal hold samples**. The new entry admission ran, and a brief middle-truck prejoin cap reduction occurred between sampled pacing records. That does not establish sustained cohesion. Unit Two separately entered the preserved native arrival recovery; its later capture cannot repair the terminal verdict.

Unit Three failed the authored route `ADVANCE_LIMIT`, then cleared its owned activity; this was **not** a native UNREACHABLE failure. At the adjacent214→215 projection, candidate station319.530 exceeded budget319.528 by0.00213623m against the existing0.001m tolerance. Do not label that a proven floating-point defect: the observed lateral offset and small turn predict almost the same projection difference, while logged coordinates are too coarse to establish the precise rejected subguard. Preserve this concrete route-transition issue for a conservative correction without granting unearned progress.

- [Independent review](../.cache/client/runs/ordinary-three-entry-pacing-v1/independent-three-review.md), SHA`E9992E6BA6AA01AA231CDA2138C1DCEC53474DB7AB0270D7357C6E3E7D21CB0D`, verifies580 sources,212 packed text resources and3 deployment files. Full console SHA`C6917AF53C24ACB9490ECD98CD1BAE4152116C224896AEE34098F7DF463BC59B`; terminal snapshot SHA`D0DB46636664D18413F5B473BE8507F76B737E04728DA8F4209A25AD4444E223`, exact prefix+171ms.
- **9 runtime /0 post-result /62 shutdown errors** remain. Natural client exit, completed watcher/recorder and fresh empty game/process inventory are recorded in `closure-review.json`.
- Private silent recording203.866667s, SHA`2121B968D8940C9D41EE564D23039BEFE16582811141D2D25A81914F1369AA34`. Two live views covered staging/travel; no full playback or ordinary-input claim.

**Product decision:** retain integrated arrival recovery and the successful one-truck supply milestone. The [two-loaded-truck workflow](convoy-supply-trip-milestone.md#two-loaded-trucks-delivery-achieved-restart-still-fails) now demonstrates loading, retained Hold and delivery; repair its failed Resume next. Preserve the spacing and route failures above. Do not launch another pacing-gain comparison simply to improve one peak number.

### Two loaded trucks: delivery and Hold work; Resume needs repair

`loaded-pair-trip-v2` loaded100 into each original truck, transported both loads over328m from the shared source, and unloaded200 into the distinct destination. All451 cargo observations conserved the same four containers: **1800/0/0/0 →1600/100/100/0 →1600/0/100/100 →1600/0/0/200**. Each native action was cancelled and followed by at least3s of stable quantities. Both drivers stayed seated with their original assignments.

Arrival completed **180.313s /180 selected-Wait samples per follower with zero measured drift**. Real server Hold then lasted **48.1325s /48samples**, covering both unloads, also with zero measured drift/speed. Original continuous driving segments were322.901m/29powered and299.130m/32powered; Unit Two genuinely followed Unit One's recorded route.

**Overall physical and strict result: FAIL.** Peak links were52.4504/81.625m, so Unit Two exceeded the unchanged60m spacing gate. Resume was accepted but failed before any fresh movement: Unit Two reported `measured_step_spans_multiple_vertices`. Its cursor had stopped updating when the moving guide retired, although the truck continued its final real-predecessor approach. Resume submitted that accumulated movement at once. The later fixture `seat_transfer_precondition` is secondary to the failed production controller; it is not solely a seat-transfer fault.

- All five configurations and packaging passed in `.cache/workbench-runs/convoy-loaded-pair-trip-v2-repair1`; pack **`037F30C6D6E18A284146536B7CEEBE6A08B547A4DE3AEC213C79A2F39701AE89`**. A preceding compile failure from an unsupported array `+=` operator is preserved separately. The exact six regression resources and generated database are canonical; `.cache/loaded-pair-trip-v2/promotion.json` records them. Production driving code is unchanged by this promotion.
- [Independent review](../.cache/client/runs/loaded-pair-trip-v2/independent-review.md) verifies585 frozen sources, three deployed files, exact four-container conservation and the terminal-prefix snapshot captured+245ms. **9 gameplay /0 post-result /64 shutdown errors** remain. Natural client closure, completed watcher/recorder and clear process/window inventories are recorded.
- The initial `loaded-pair-trip-v1` failed its loading stationary guard at0.827s before any native transfer. The exact speed-versus-drift term was unlogged. V2 waits for both trucks to settle before pinning origins; successful native loading now validates that preparation. V1 remains preserved with9 gameplay/0post/68shutdown errors.
- [130-second game-view highlights](../.cache/test-videos/loaded-pair-trip-v2-highlights.mp4) use raw intervals70–145,185–200 and398–438s. The private full recording is485.533333s, SHA`6650521EAF1E18BB6A1AF51E68C59337AD9DC47B2341A32F700F0B40679BCBA5`. Silent; four live views and one recorded frame were inspected, not full playback. This uses test-owner staging and a native AI lead; ordinary input remains unproved.

### Resume candidate: arrival reversal exposed a second defect

The private `loaded-pair-trip-v3` candidate passed all five configurations and packaging (`CDE24B2F16E895335721FB32B8C2E7349966367F5FCBE60CB315F6DE5DD3639D`) but failed its live trip before unloading. It retained actual route observation during owned arrival and added a bounded seated wait for Resume. Both trucks loaded100 and travelled; the recorded route cursor advanced during the final approach, resolving the stale-observation mechanism seen in v2. However, Unit Two reversed during native arrival and the retained route query returned `OFF_ROUTE`. The controller failed at139.074s, before formal arrival observation, explicit Hold, unloading or Resume. Peak gap54.4586m does not turn this into a trip pass. The candidate remains private.

The run also exposed an unnecessary replacement of the recovery approach: sequence4 was issued at17:28:54.634, then replaced by sequence5 at17:29:01.600 while the predecessor remained parked. Source inspection found that recovery had not refreshed the predecessor position used by the still detector. The next narrow comparison refreshes that anchor and restarts the timer at recovery handoff, preserving the single approach and all capture/route gates. Whether this prevents the reversal is unproved; an explicit new departure phase remains a possible later design if parking legitimately invalidates the old travel route.

The [independent closed-run review](../.cache/client/runs/loaded-pair-trip-v3/independent-review.md) verified585 frozen sources, three deployed files,179 conserved cargo rows and an exact terminal snapshot captured+145ms. It retains9 gameplay/0 post-result/64 shutdown errors. Client, watcher and recorder closed normally; the226-second silent video was sampled in two live views and one recorded arrival frame. Compile-only repairs remain separate. Keep the working v2 delivery and canonical controller checkpoint. Do not promote this failed candidate or substitute command acceptance for restart movement.

### Repeatability changes the next priority

`loaded-pair-trip-v4` added only the two recovery-handoff assignments to v3. It passed all five configurations and packaging, pack `034B5BBABBD49586876F2E4245E7B6BD428EE221AB220CC19CDD25CDB3B5E415`, but failed **during active travel**, before any arrival-recovery handoff ran. Unit Two reversed under its exact guide activity, the retained route cursor stayed at58.4027m, and the active route query returned `OFF_ROUTE` at17:39:19.354. Spacing had already failed; final peak was67.7834m. Both100-supply loads were retained, but no arrival observation, Hold, delivery or Resume completed. The handoff repair is therefore **unexercised**, not disproved or demonstrated.

The [independent v4 review](../.cache/client/runs/loaded-pair-trip-v4/independent-review.md) verified585 frozen sources, three deployed files and120 conserved four-container rows. The exact terminal prefix was captured+189ms. Natural closure, watcher and recorder completion are verified;9 gameplay/0 post-result/64 shutdown errors remain. The silent151.266667-second recording preserves the failure; visual review comprised loading and the post-terminal road scene, not full playback.

This is now a repeatability problem in normal following, not just the end-of-trip Resume transition. A route cursor that only advances can reject native reverse recovery even when the truck remains near its previously driven road. Investigate bounded physical backtracking/reacquisition and a controlled real-predecessor fallback without erasing history, inventing a join, skipping across a hairpin or redirecting every truck to the player. Exact failure geometry must distinguish backward motion from lateral deviation; the current samples do not establish the precise failing segment.

**Next product step:** implement and check [bounded reverse recovery](convoy-reverse-recovery-plan.md), then use the existing native lead in a short, clear-road two-truck stop/Hold/Resume comparison before another complete cargo-route retry. Reuse existing components rather than building another large coordinator. Preserve all failed full-trip gates, original assignments and genuine movement evidence; a focused pass is not a full delivery pass. Return to the filmed real-map supply trip after the core transition works. Keep v3/v4 private and the canonical delivery checkpoint intact.

### Reverse tracking: useful travel, arrival remains the blocker

The private reverse-recovery candidate preserves a separate actual route station and monotonic high-water mark, permits locally proved backward movement, and continues route observation during the owned arrival and Hold. It does not reset the original join or widen the 5 m route corridor. All five configurations and packaging passed. The native geometry run passed 93 cases initially and 97 after a clamped-vertex correction; these are route-helper results, not a convoy-trip pass.

The focused course reuses the existing two-truck trip coordinator and native lead, explicitly skips cargo, and retains the physical travel, spacing, arrival, Hold and Resume requirements. Its setup comparisons are preserved:

- `reverse-recovery-stop-resume-v1` failed initial route-entry geometry before tail travel. This also exposes a real limitation when recruiting trucks from arbitrary offsets.
- V2 was packed but **never launched**: review caught that its shortened goal contradicted the inherited minimum lead distance. Its desktop recording is not gameplay evidence.
- V3 restored the proven full-trip starting positions and native goal. Both followers travelled over 309 m, with peak gaps below 60 m, but the tail rejected a local reverse query before arrival completed.
- `reverse-recovery-stop-resume-v4`, pack `1DAD97F423199AB93F473B6172AA6BAD67AC8B207CC18883CA3D791694A092FF`, passed genuine predecessor-route joining and useful travel with a 56.6003 m peak link. The tail accepted five tiny backward station corrections, totalling 0.00585938 m; those are not powered movement credit. At 18:17:44.796 it transitioned to the ordinary real-predecessor arrival approach. It failed at 18:17:46.112 with `reverse_corridor_lost`, before formal arrival observation, Hold or Resume. The label also covers a residual-projection check, so it does **not** establish a physical departure beyond the corridor. Original identities and the failed result remain preserved. The terminal snapshot was captured +66 ms; the client exited naturally and the watcher/recorder closed.

The [v4 independent review](../.cache/client/runs/reverse-recovery-stop-resume-v4/independent-review.md) verifies all 588 frozen sources and three deployment files, movement of 323.795 m /36 powered intervals and 309.21 m /33 intervals, and natural closure. It retains **9 gameplay /0 post-result /64 shutdown errors**. Only the departure was captured live; the silent full recording is preserved, not fully reviewed.

Keep these candidates private and the working canonical controller intact. A concrete clamped-corner example now demonstrates that the orthogonality-only refusal conflicts with accepted route coordinates, even without floating-point error. The next candidate removes that redundant refusal while retaining the corridor, signed motion, actual measured displacement and local-history constraints. The exact historical failure frame remains unproved. Repeat the physical stop/Resume comparison, then return to the existing complete loaded trip. Do not expand the course or add unrelated polish while this loop remains incomplete.

### V5 correction checked; separate straight-stop course next

V5 removed only the inconsistent orthogonality refusal and added one clamped-endpoint reverse regression. All five configurations and packaging passed; the native route fixture passed **98/98**. Pack: `44B9394EDA8B148766AB64B6A520B4BA11E8CC35D314027DCFAE386F0E4076CD`.

The same physical junction course still **failed**. Both followers travelled over 314 m, but Unit Two first exceeded spacing at 55.19 s, peaking at **81.7859 m**, then its moving route query rejected `local_turn_over_90_degrees`. This was a different failure while still under its moving guide, not proof that the corrected reverse guard failed. No formal arrival observation, Hold or Resume completed. Twenty-seven accepted reverse station corrections totalled 0.0492439 m; they are not driving credit. The [independent review](../.cache/client/runs/reverse-recovery-stop-resume-v5/independent-review.md) verifies 588 sources, three deployment files, the exact terminal prefix captured +13 ms, natural closure, and **9 gameplay /0 post-result /64 shutdown errors**. Departure and the junction were inspected live; the full silent recording remains private.

**Product decision:** stop refining route mathematics in isolation. Preserve this difficult junction control and test the basic stop/unload/resume workflow at a suitable stopping place. The next private course uses the natively resolved connected point `<1598.08,20.7374,3363.82>`, approximately 229.5 m from the established start and before the junction. It preserves V5 production code and the physical acceptance gates; its declared restart leg is 45 m to stay on that road segment. This is a changed fixture, not a pass or repair of the old junction case. If stopping and restart work, repeat actual native loading and unloading there and record the usable trip before further polish.

### Straight stopping point: spacing holds, tracking still blocks arrival

`straight-arrival-stop-resume-v1` ran the declared pre-junction goal with unchanged V5 production code. Five-configuration validation and packaging passed: **`497B6E8806966A7B573F80C2148A0F1D6CAA5A58CA413DB6236EEA28563F2039`**. All 588 frozen sources and three deployment files match; only the explicit-goal selector, restart-length parameter and scene layer differ from V5.

The physical run **failed** at 18:37:39.845. Exact moving activities produced **239.953 m /28 powered intervals** and **212.349 m /23 intervals**. Peak link was **56.635 m**, but Unit Two rejected `reverse_previous_segment_unavailable_or_step_spans_vertices` at 18:37:39.195 while still under its moving guide. Unit One captured arrival three seconds after terminal failure; this cannot establish a completed two-truck arrival. No formal Hold or Resume ran. The [independent review](../.cache/client/runs/straight-arrival-stop-resume-v1/independent-review.md) preserves **9 gameplay /0 post-result /64 shutdown errors**, the full log and exact terminal-prefix snapshot captured +135 ms. Natural client closure, completed watcher/recorder and a fresh empty game-process check were verified. One live travel view was inspected; no full-playback or normal-input claim.

This isolates another tracker rejection on a reasonable stopping course. Preserve the failed junction and straight controls. The immediate work remains practical handling of small local reverse/arrival motion, not new features or a lower acceptance bar. Complete stop/Hold/Resume here before another loaded delivery attempt. The exact candidate source is now preserved as a reviewable [experimental patch](experiments/straight-arrival-v1.md), separate from the working canonical addon, so continuation does not depend solely on a local cache.

### V2 endpoint correction: handoff lifecycle is the next blocker

`straight-arrival-stop-resume-v2` used the exact V1 course and physical gates, changing only the endpoint-retreat helper and two native geometry cases. All five configurations and packaging passed, pack **`2D7E438A93055018E9982F61CDFC359E4BD982F8C453C0A3A45AC801579F62A0`**; the separate native fixture passed **100/100**. These helper results do not establish successful convoy driving.

The physical run **failed** at 18:54:36.254. Spacing first exceeded 60 m at 18:54:07.039, ultimately reaching **96.0326 m**. At that first crossing, the tail was recovering from native braking while its predecessor was travelling faster. This is separate from the later route failure; its engine cause is not established.

The tail's guide was retired and a real-predecessor arrival request was bound at 18:54:35.754. It failed at 35.971 with `reverse_physical_context_unavailable` before that new request activated. The first logged incoming-vehicle avoidance transition was later, at 36.205, so it does not prove avoidance caused the context loss. No formal arrival observation, Hold or Resume completed. Preserve the full log, exact terminal snapshot (+228 ms), frozen sources, private recording and [independent review](../.cache/client/runs/straight-arrival-stop-resume-v2/independent-review.md), SHA `C28727DC9DD26B65EBF3B2CA8A59CC27FB2E08F99C2968585EBFA6C0B8CBA5B2`. **9 gameplay /0 post-result /64 shutdown errors** remain. The client closed naturally; watcher and recorder completed and a fresh process check found no remaining game or recorder. Two live views covered travel and approach, not full playback or normal player input.

**Next practical comparison:** retain read-only actual-pose observation while a correctly bound owned arrival request is pending, with driver/truck/predecessor identity and player-control protections intact. Issuing a movement request still requires the existing execution checks. This tests the handoff lifecycle without another route-mathematics change. Keep the original spacing and cargo requirements, and return to a complete filmed supply trip when the transition is dependable.

## Preserved failed controls

| Closed comparison | Result and useful boundary |
| --- | --- |
| [Trip v2, 25 km/h mixed pair](../.cache/client/runs/convoy-trip-two-v2/original-tail-independent.md) | Links 76.0872 / 63.9199 m fail spacing; guide → real predecessor → owned Wait arrival succeeds; 180.313 s zero drift |
| [Direct pair with head rear pacing](../.cache/client/runs/original-direct-rear-two-v1/paced-independent-result.md) | Links 83.8187 / 86.7728 m fail spacing; 180.346 s zero drift; simpler direct following is not a spacing fix |

Earlier route, native-request, explicit-Hold, Resume and input failures remain in the [validation history](convoy-validation-history-2026-09-26.md) and exact run directories. Do not attribute improvements to one change when timing, pace or multiple mechanisms differed.

## What is still unproved

- **Ordinary input:** supported desktop activation/click/key delivery remains unresolved, including vanilla controls. Capture and scripted engine-action calibration are not keyboard/panel proof. Elevated OSK accessibility exposed no supported Close action; its causal role remains unproved.
- **Cargo:** native load, transport and distinct-destination unloading are now verified for one and two ordinary followers in automated fixtures. Only the one-follower case completed powered Resume; the two-follower restart fails. The ordinary cargo menu/input path remains unproved. Retain native eligibility, exact container counts and conservation; no count setters or transfer fallback.
- **Complete player workflow:** the [Supply Day scene](convoy-supply-showcase.md) is prepared, not a completed recruit/load/travel/unload/Resume demonstration.
- **Wider driving:** repeatable ordinary one-, two- and three-follower trips, turns, longer sessions, sustained grass/off-network travel and varied stops remain required. Moving dirt contact is now measured for all participants on the mapped mixed course; a scene survey alone remains insufficient.
- **Interruptions and roles:** selected-member Hold with downstream waiting, rechain, predecessor/driver loss, possession, blocked recovery, unload/return and regroup need relevant ordinary-driver evidence. The private deferred-timeout recovery still requires StandDown/reassignment.
- **Multiplayer and release:** authority/ownership, joining and reconnecting players, USSR behavior, configuration, performance, distribution and a finished demonstration remain open. A first supply trip is an intermediate milestone.
- **Errors:** physical successes retain all runtime and shutdown failures. Matching vanilla signatures establishes independent occurrence, not harmlessness, ownership attribution or a waiver.

## Next usable-trip actions

1. Preserve the demonstrated ordinary-driver baseline; address concrete supply-trip blockers before broadening driving experiments.
2. Repair route-following recovery and the two-truck stop/Resume lifecycle in a short visible native-lead course, then rerun the existing complete loaded trip. Preserve per-unit movement/Hold/Resume proof and conservation across all four containers. Keep the failed pacing and v3/v4 candidates private and the concrete adjacent-projection fault visible; no threshold relaxation or automatic tuning loop.
3. Resolve ordinary input through a supported, discriminating control; do not repeat unchanged key/click matrices or treat capture as input success.
4. Complete and record the ordinary Supply Day workflow with retained assignments, useful status and one spokesperson. Extend to two/three followers and realistic interruptions.
5. Continue the full roadmap: verified unpaved routes, regroup/return, role changes, Soviet and multiplayer coverage, performance/configuration and release polish.

## Evidence discipline and navigation

Keep the existing **60 m whole-run per-link peak**, exact original identities/predecessors, continuous powered travel and **180 s / 2 m** paced-hold gates. Explicit Hold requires **30 s** and Resume a fresh continuous **20 m / three powered intervals**. Command acceptance, projected route station and telemetry requests are not physical completion.

Preserve the complete console, terminal-prefix snapshot, frozen source/deployment hashes, physical and error verdicts, closure evidence and visual-review limits. Natural exit and a completed video do not erase errors. Raw full-desktop videos remain private.

- [Focused courses](convoy-feature-courses.md) · [Navigation/release plan](convoy-navigation-release-plan.md) · [Edge cases](convoy-edge-case-matrix.md)
- [Operations playbook](operations-playbook.md) · [Harness handoff](harness-handoff.md) · [Full historical validation](convoy-validation-history-2026-09-26.md)

Historical documents preserve dated observations. This checkpoint owns the latest result and next boundary; the full brief owns product scope, and the latest user instruction owns execution authority.
