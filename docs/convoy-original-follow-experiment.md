# Original native Follow orchestration

This is a private driving experiment, not shipped behavior or a release claim. Normal driver prefabs still use the production controller. The authored graph references native tasks but does not copy their implementation. Older copied-tree comparator resources remain in these private packages and must be removed or cleared before public distribution.

## Question and boundary

Earlier entity-follow controls could brake, reverse or lose spacing while the same activity remained selected. A trail-guide adapter did not fix that behavior. This experiment separates movement orchestration from route history: target the actual predecessor with an authored graph, retain native vehicle steering, and omit the copied general movement-planning/failure subtree. Keep the same road, vehicle poses, 25 km/h lead request, 20/10 m moving/stopped distances, lead MOVE radius 5 m, and captured Wait. Rear pacing is disabled.

An immutable order lease binds the original driver, truck, group, session, waypoint, predecessor and activity. Admission requires settled vehicle registration and no competing boarding work. Invalid ownership blocks the request rather than issuing an implicit seat order. Deferred retirement must keep a new order from overwriting an occupied waypoint slot. These safety paths still need interruption and possession tests.

## One follower: physical PASS, strict full-run FAIL

Existing five-configuration validation and packaging succeeded for `.cache/packed/convoy-original-follow-v2`, data SHA-256 `424A3383A06DB504106ED7CE6BE597B5BB3DA96156548366FF22BB15942830EB`.

The completed run `.cache/client/runs/paced-original-follow-1truck-v1/` retained the original pilot, truck and direct predecessor. Peak gap was **49.2195 m**, below the unchanged 60 m gate. One exact moving activity covered **320.568 m with 32 powered samples**. Both vehicles held for **180.364 seconds**, with 180 observations and zero measured drift; the follower's exact captured Wait remained selected throughout. The matched `5EA78318...` control had failed spacing at 83.192 m. This is one comparison, not proof of general superiority.

Six actual reverse episodes remain near arrival. Two failed lease callbacks occurred 17–18 ms after normal controller-clear revocation; no new requests followed those revocations. Keep that lifecycle defect distinct from a native request failing during travel. A sticky failure flag can also make a later Resume appear accepted without a fresh order; a separate repair and command test are required before promotion.

Natural client closure occurred after 30.0159 seconds of post-terminal observation. **Nine runtime and 64 shutdown errors remain**, so the strict full-run result is FAIL. All runtime signatures have saved exact vanilla matches; none was waived. The independent report verifies the three deployed files, 533 frozen source entries, and 179 supported packed text resources.

- [Independent result](../.cache/client/runs/paced-original-follow-1truck-v1/paced-independent-result.md), SHA-256 `12787755518DB3D52571A01D4CBB84522F183E0479164EAB64643B17FA94D54B`.
- [Detailed driving comparison](../.cache/client/runs/paced-original-follow-1truck-v1/driving-comparison.md).
- Full console SHA-256 `4DFC290D9FF8B2A831DF004596F871B65D24EC395A88902A8E5AF1C20199A885`.
- Exact-prefix terminal snapshot SHA-256 `16C9DE062E27A2532DF40A8330186D1B460EEBB9E4B0564DD03FD5C512C5A5D7`, captured 82 ms after the marker.
- Private recording `.cache/test-videos/paced-original-follow-1truck-v1-full.mp4`: 393.733333 seconds, 5,905 frames, no audio; SHA-256 `F117E15B3F793867CC41F9BABFDB5DE4CA1D1D0C453A7A1CE8FC0A8F62BFE3A0`.

Visual review covers a live predeparture capture, a stored frame at 65 seconds, and six cropped samples from 35–335 seconds. Later samples show the pair aligned at the stopped road approach. This is sampled coverage, not full playback or a smooth-driving demonstration. Keep the full desktop recording private.

## Two followers: physical FAIL, strict full-run FAIL

The new `OriginalFollow_2Trucks` world retains the earlier two-follower geometry and all gates, replacing only the two character slots with the authored-graph variant and assigning distinct resource IDs. The scripts and graph are unchanged from the one-follower run. All five script configurations and packaging passed as `.cache/packed/convoy-original-follow-v3`, data SHA-256 `223CA993D3915D9805586149F63AC9352463F75563FFF61DBEA890B371A18B83`.

The finalized [independent report](../.cache/client/runs/paced-original-follow-2truck-v1/paced-independent-result.md), SHA-256 `3413BB8AA3F04B870D5E4AC08D328073A3F570F2186D47E2DE91553C96EFC89E`, verifies the complete 536-file source snapshot, 181 supported packed text resources and three deployed files. Unit One/Two peak gaps are **46.0147 / 108.378 m**, so spacing fails. Original pilots, trucks, memberships and immediate predecessors survive; each follower exceeds 100 m and three powered samples under an exact activity. All three vehicles completed **180.312 seconds / 180 samples with zero measured drift**, with both follower Waits selected throughout. The whole course remains failed despite that hold.

Natural closure followed 30.0164 seconds after the terminal marker, with cleanup and `Game destroyed`. **Nine runtime / zero post-result / 64 shutdown errors remain unwaived.** The snapshot is an exact prefix captured 10 ms after the marker. Full console SHA-256 `8CF2629183EC72FCA6AA71ADE285EB88CFF1E42A3B13BCAC798059DEDF6567C2`; snapshot `3604F842EF7B24C8406F4B9BD7BE0E427E6E428481348A5734D74FBD97A6D2D4`. Private recording `.cache/test-videos/paced-original-follow-2truck-v1-full.mp4` is 377.6 seconds / 5,663 frames, SHA-256 `B95CFBCC34895824EE26B291C2C55C299792937807EC915BCC41A973C8C4F3A0`. Two live captures and eight stored frames from 45–80 seconds were reviewed; full playback was not performed.

The saved [spacing diagnosis](../.cache/client/runs/paced-original-follow-2truck-v1/spacing-diagnosis.md) identifies an uphill native brake/near-stop while the tail's same exact predecessor request remained open. The observed returned-path fold does not prove its underlying cause. Preserve this failure and the original raw artifacts; do not extrapolate the one-follower pass to two or three followers.

The independent report is complete. The subsequently integrated private command repair, four-file Hold/Resume fixture and parked-lead policy compiled as `FA913F7A...` and completed `original-follow-hold-resume-v1`. Its bounded baseline passed at 52.6805 m peak gap with a 180.364-second zero-drift hold, but actual Hold retired the arrival Wait and failed physically. Resume was not reached. Natural closure retains nine runtime and 62 shutdown errors. [Current validation](convoy-validation-current.md) records the exact report, diagnosis and provenance. The user has now explicitly resumed after documentation setup; the next command change must address that ownership gap, independently of the reviewed, still cache-only tail-truck route-guide comparison. Keep failed evidence and command/movement changes separately attributable. Normal keyboard/panel use, supply transfer, multiplayer, varied routes, unpaved coverage and public integration remain open.
