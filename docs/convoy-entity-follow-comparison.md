# Persistent predecessor activity: first physical comparison

September 26, 2026. This is an isolated prototype result, not a production promotion or release pass. The [current validation record](convoy-validation-current.md) remains authoritative for later results.

## Why this experiment

The ordinary MOVE baseline repeatedly changed waypoint destinations. Its one-follower run reached a 140.223 m peak separation and reproduced a backward native path connector. Retaining fixed MOVE destinations avoided that particular reverse in a separate two-follower comparison but made gaps worse through completion, Idle, and restart cycles.

This prototype keeps one native vehicle-enabled Follow activity bound to the actual predecessor vehicle. Normal convoy prefabs and the base controller are unchanged. It tests activity continuity; targeting the predecessor's current vehicle does not establish that a truck follows its actual driven trail through arbitrary corners.

## Frozen comparison

- Candidate world: `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_EntityFollow_1Truck.ent`.
- Candidate package: `.cache/packed/convoy-entity-follow-v2`, data SHA-256 `21658B5401A8FF709BE5AC031FA78F78F35183E910782CAABE051A1074DABBCE`.
- Evidence: `.cache/client/runs/paced-entity-follow-1truck-v1/`; full private video: `.cache/test-videos/paced-entity-follow-1truck-v1-full.mp4`.
- Control: `paced-native-close-1truck-v1`, frozen data `CF7B2B778F56A3BF0DBCCE500B8657A0AFFE2CE9E111B1EB31C53AD151F2B680`.
- Both use one follower plus the native lead, moving/stopped gaps 20/10 m, MOVE radius 5, native cruise enabled, stable-waypoint retention disabled, and the same requested lead cap of 25 km/h. The actual lead can exceed that requested cap.
- Gates remain original identity/seat/predecessor, powered signed progress, peak separation at most 60 m, and 180 seconds of observation with at most 2 m drift. The prototype additionally requires powered progress tied to one exact persistent activity and retirement of that activity at arrival.

The first script validation rejected a diagnostic variable named `owned`. Renaming it to `waypointActivity` passed WORKBENCH, PC, XBOX, PS4 and PS5 validation and packaging. This was a compilation repair, not a driving change.

## Observed result

**Overall physical FAIL; strict full-run FAIL.** Moving behavior improved, but arrival did not satisfy the hold requirement.

Peak separation was **50.7126 m**, below the unchanged 60 m gate. The original truck, pilot and predecessor remained intact. One persistent native activity carried more than 300 m of independently measured signed progress with engine, forward-gear and throttle evidence. At the former uphill reversal site, the follower crossed in forward gear with brake zero; its actual native car path continued ahead. The unchanged waypoint origin is an ownership anchor, not evidence of a stale native path.

The follower then stopped at roughly **20.16 m** behind the lead. At 05:54:41.971 the ordinary-arrival fallback replaced Follow with a fixed MOVE to close the gap. That restarted the truck and produced reversing at the junction. The later 180.379-second observation retained all identities but measured **5.28124 m** maximum follower drift, exceeding 2 m; lead drift was zero. All 180 observation samples had no live private Follow activity or private waypoint. The failure is downstream of the fallback, not an unretired Follow order.

The inherited preliminary arrival check samples net displacement every five seconds, so oscillating motion can appear settled. The stricter continuous observation correctly rejected this run. Preserve that failure rather than moving the observation start to hide it.

Native closure succeeded: the fixture requested exit 30.0163 seconds after its final FAIL, cleanup and `Game destroyed` followed, and the client exited naturally with code 0 before the safety timer. **Nine runtime errors and 64 shutdown errors remain**; natural closure is not an error-free lifecycle. The terminal snapshot is an exact full-log prefix captured 96 ms after the marker.

The private recording is 332.8 seconds / 4,991 frames. Live captures and extracted frames at 60, 110, 140 and 280 seconds were inspected. Those show road travel, junction approach and later stationary poses, with a thin on-screen-keyboard border; they are not full playback or independent velocity measurements. Moving wheel samples cover mapped dirt road and later asphalt, not an entirely unpaved or off-network journey.

## Subsequent stopped-entity comparison — preserved evidence

The proposed shorter-distance Follow comparison has now run in the saved evidence. Frozen package `E01A1EC379C84C2F2E0BB205CD5AB5C93D695444CAC797FB9768CDFBD732287F` kept the moving activity, issued one 10 m same-predecessor replacement, and retired it only after measured capture within the 14 m latch with the existing brake/cruise owner. The [independent report](../.cache/client/runs/paced-entity-stop-1truck-v1/paced-independent-result.md) records physical PASS at 52.0785 m peak gap and 180.313 seconds / 180 samples per truck with zero hold drift. Four follower reverse episodes remain. Strict acceptance failed with nine retained runtime errors and unobserved normal shutdown after an early native-close callback and later forced termination.

The newer `7C599AE1...` package passed five-configuration validation and packaging. Its completed two-follower run (`paced-entity-stop-2truck-v1`) failed physical and strict acceptance: peak links 55.1046 / 76.1319 m and Unit Two drift 7.57021 m over 180.476 seconds. Both original pilots, trucks and predecessors survived, with independently powered progress under continuous native activities. Unit Two's selected incoming-vehicle reaction resumed 0.302 seconds after capture, despite retirement of the private Follow order. Natural close completed; nine runtime and 62 shutdown errors remain. The three-follower extension remains compiled-only.

Explicit Hold, restart, smooth arrival, varied turns, actual driven-route guidance, human input, multiplayer and production integration remain separate gates. Preserve each run and its unchanged physical thresholds. The current user instruction permits documentation setup only; further comparisons await an explicit update and resumption.

## Subsequent captured-Wait comparison — preserved evidence

Frozen package `D4E3874CE7928747E061F64653866B8302E1E2950658EA95C173A997B79154CC` passed five-configuration validation and packaging after a sequential-guard compilation repair. Its [two-follower report](../.cache/client/runs/paced-entity-wait-2truck-v1/paced-independent-result.md) records **physical and strict FAIL**: Unit One / Unit Two peaked at 52.1781 / 70.0129 m, against the unchanged all-link 60 m gate. It retained original trucks, pilots, predecessors and continuous powered travel.

The isolated hold intervention passed: all three trucks stayed within zero measured drift for 180.676 seconds / 180 samples each, with the exact owned Wait selected throughout every follower sample after private Follow retirement. Priority 2100 deliberately suppresses the previously observed incoming-vehicle reaction. This establishes the private stationary experiment, not safe dynamic threat handling, restart, smooth arrival or production promotion. The four pre-capture Unit One reverse episodes remain. A smaller moving gap cannot be attributed to a later stationary intervention.

Natural closure completed; nine runtime and 64 shutdown errors remain without waiver. The exact terminal snapshot, full log, source/deployment hashes and private 398.266667-second recording are preserved with the report. Visual review covers three live captures, not full playback. Newer default-off rear-pacing and separate callback-action eligibility source remains uncompiled and untested. Preserve it and verify final manifests/reviews after resumption; do not transfer this pack's results to newer source.

## Source preservation and distribution

The local prototype manifest and provenance are `.cache/entity-follow-prototype-files.json` and `.cache/entity-follow-prototype-plan.md`; each run also preserves its source manifest. The private activity tree derives from an installed Bohemia tree. Local experimental use is not a claim of general redistribution rights. Keep that tree out of public commits until the distribution basis is established or an original orchestration graph using native references is independently implemented and validated. Frozen local evidence remains preserved.
