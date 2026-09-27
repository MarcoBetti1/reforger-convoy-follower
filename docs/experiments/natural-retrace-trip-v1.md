# Supply trip with bounded predecessor retrace recording

**27 September 2026 — closed FAIL, with native delivery and actual renewed travel.** Both original trucks delivered their100 supplies, completed arrival and Hold, then moved about50m and captured arrival again. The unchanged formal Resume gate failed because the tail switched to its real-predecessor stopping approach before earning20m in one eligible guide sequence. The61.0379m spacing failure also remains.

| Gate | Observed result |
|---|---|
| Native cargo | 100 loaded and unloaded per truck; destination0→200 |
| Conservation | 540 terminal /570 full-log original-container records; total1800, no discrepancies; final1600/0/0/200 |
| Arrival | 180.296s /180 exact captured-Wait samples per driver, zero drift |
| Explicit delivery Hold | 51.099s /51 samples per driver, zero drift; both negative Resume calls safely rejected |
| Fresh Resume proof | Head sequence7 qualified24.58012m/8 powered intervals; tail sequence8 earned16.157m/8, below20m |
| Actual final movement | Head net51.3224m /tail50.0396m; both subsequently captured arrival |
| Spacing | Head61.0379m /rear40.5539m sampled peaks; maximum through terminal61.0379m fails60m |
| Native geometry /errors | 73 cases passed;9 gameplay /0 post-result /64 shutdown errors retained |

## What executed

The existing bounded **arrival-recovery handoff** ran. A real tail move3 failure was admitted at03:42:47.738 with23.9461m gap. At03:42:57.771, the predecessor's exact selected arrival Wait, unchanged stop episode and measured8.97891m advance admitted the32.7727m handoff gap; the advance remained below the declared10m bound. One fresh native approach followed, and the tail captured arrival at03:43:16.871. This is separate from the fixture's explicit natural-failure Hold/Resume path, which was **not exercised** (`natural_failure_count=0`).

The new tiny-retrace recorder produced **no live buffer events**. Its six added native cases passed alongside67 existing cases, but this run does not prove that branch repaired the [previous departure rejection](natural-recovery-trip-v1.md). No trail block occurred in this comparison.

At03:47:55.271 the tail changed from guide sequence8 to exact real-predecessor10m approach sequence10. The observer requires the tail's1m guide target and TRACKING state for moving credit; it resets progress on this transition. The subsequent direct approach physically carried the tail to its new stop. Summing the two activities or treating total net travel as20m/3 powered proof would bypass the declared gate. Head qualification was independently reproduced, while terminal03:49:00.437 retains `pair_complete=false`, `workflow_complete=false` and overallFAIL after the90s restart timeout. No course extension or acceptance change was made to obtain a green result.

## Reproduction and inspection

[Four-resource patch](natural-retrace-trip-v1-source.patch) and [manifest](natural-retrace-trip-v1-source.json) apply after the exact [natural-recovery source archive](natural-recovery-trip-v1-source.json): two private recorder/caller product files, the geometry probe and its inert attachment in the existing world. Physical query and trip gates are unchanged. Isolated application reproduced all four byte hashes with no extra paths;590 other files matched the baseline. Frozen594/594 and deployment3/3 verified. World: `Worlds/Tests/ConvoyFollower_Arland_NaturalRecoverySupplyTrip_2Trucks.ent`; pack `083875C2BEB6D07E462B47162771D97C16E7AC84F62E5607B092D811879B7600`. The earlier duplicate-Reset compile failure was never launched.

Terminal capture was an exact prefix at+100ms. Client, watcher, video and audio closed naturally with exit0; empty process/window checks were retained. Video593.8s and audio605.7s hashes were independently verified. Four live views included arrival/unloading, renewed movement and terminal+10.592s. Full playback and audio content/synchronization were not reviewed. Raw output-mix media is not published here.

This assisted native-AI/server-command run demonstrates useful supply delivery and renewed travel, not a qualified complete loop, ordinary-input validation, multiplayer or release readiness. The exercised arrival handoff and unexercised recorder change have separate evidence boundaries.

## Product integration and demonstration

After closure and independent review, the exact exercised ordinary-controller file `19B49FBF927CA32826F620E11574C451D015B9AD961FD303D8EE6AA980BF048A` was promoted over `DB1255D5…5A0284`. The main addon retains its newer command guidance. Five-configuration canonical validation and packaging passed as `3CC40540C30BE9CAEF37499424809B57DD7A3C8B3C7340EF4AC6B91CAB706F1D`; this is a different composition from the experiment. The retrace recorder, its caller change and private fixture resources were not promoted. `.cache/arrival-handoff-promotion-v1/promotion.json` preserves attribution and prior bytes.

The [143-second silent development video](../../.cache/test-videos/natural-retrace-trip-v1-demo.mp4), SHA-256 `A6495098843C497CD640128C0FEBE83424930F5B0C57969F26C8C48F5FC8D4AE`, shows the useful supply trip and discloses automated controls and unmet gates. Root inspected its twelve-frame contact sheet and full-resolution final caption. This is sampled visual review, not full playback or audio verification. The edit uses only the game crop; full desktop and output-mix originals remain private.
