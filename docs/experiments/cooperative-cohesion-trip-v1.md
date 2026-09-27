# Cooperative leader, loaded Road81 trip V1

September 27, 2026 — **closed FAIL; private, not promoted.** The leader consumed the same authoritative cohesion advice exposed to the owner. It cooperated by slowing below a20km/h ceiling; this does not prove constant-speed following is repaired.

| Stage | Observed result |
|---|---|
| Native cargo |100 loaded into each original truck. All220 full-log ledger rows conserve1800; final source/cargo1/cargo2/destination1600/100/100/0. **No delivery.** |
| Powered travel | Lead258.3810m/44 powered intervals; head258.034391m/50 in originalseq1; genuinely joined tail234.336190m/38 in guide seq2. Exact assignments retained. |
| Adjacent peaks | **41.7299m head /41.6725m tail**, below60m during the observed travel. The aborted course does not establish a complete spacing pass. |
| Arrival and commands | Head captured arrival; tail remained in recovery Wait. One formal observation sample thenFAIL.180s arrival, real Hold, unloading and Resume were not reached. |
| Full errors | **9 gameplay /0 post-result /62 shutdown**, retained. |

## What improved, and what failed

The first advice warning occurred at an actual38.5583m gap, before60m, and reduced the cached lead cap20→8.5094km/h. Across124 samples (103 valid/21 unavailable), invalid advice never increased the cap, accepted advice was fresh, and upward release stayed within3km/h/s.99 next-poll native-cruise readbacks matched the preceding cache within the existing1km/h tolerance; the same owned lead MOVE consumed it. Native goal Wait then held0. Three warning records do not prove audible radio playback. Level2 WAIT advice, stale-data injection, explicit Hold/Resume consumption and the retired-action deferral remain unexercised.

Head peak was much smaller than the [canonical pending-guide comparison](canonical-pending-guide-trip-v1.md)'s169.795m (tail42.5945m), but this run ended before the complete command loop. At01:57:31.585 the tail's native request failedUNREACHABLE and entered bounded recovery at21.8308m. The head finished its arrival farther ahead; final gap31.2873m exceeded the existing30m recovery-handoff check. Tail never acquired a captured arrival Wait. The fixture began observation at01:57:57.803 and correctly rejected missing tail capture one second later: `exact_captured_wait_lost_or_replaced_unit_2`, `pair_complete=false`. That generic label is not evidence that a previously captured Wait was lost. The next useful repair is the bounded handoff after the same predecessor completes its arrival maneuver, with ownership and physical gates retained.

## Reproduction and evidence

World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`. Reuse the frozen package and matching resource database from `.cache/standalone-cooperative-cohesion-trip-v1-addons`; all-five validation/packaging passed. **585 frozen sources /3 deployed files** independently verified, exactly nine declared changes over the preceding frozen course: six product advice files and three opt-in fixture resources; five driving files unchanged.

- Data SHA: `EC0374C6AD4A9446B9AB0DB3F6D19BF62E28050602A44AE33B17FAB92CF24007`.
- Manifest SHA: `2FAFF80412A87E0EFD97CEEF2DF66F5E6DDE40C7BDEF368592605E236F3FEA19`.
- [Independent review](../../.cache/client/runs/cooperative-cohesion-trip-v1/independent-review.md), [machine analysis](../../.cache/client/runs/cooperative-cohesion-trip-v1/independent-review.json), [full console](../../.cache/client/runs/cooperative-cohesion-trip-v1/logs/console.log), [closure record](../../.cache/client/runs/cooperative-cohesion-trip-v1/root-closure-video.json).

The terminal snapshot is an exact full-log prefix captured+230ms. Client, watcher and recorder naturally closed with exit0; subsequent process/window checks were empty. The [silent raw recording](../../.cache/test-videos/cooperative-cohesion-trip-v1-full.mp4) is262.066667s/3930frames, independently hash-verified (`812155715480BB33FEEB2CB6973871DE6EE2553C72E795D63D109A8AF3BA51DA`). Root sampled three live views, including post-terminal; no full playback or exact-terminal screenshot. Native scripted cargo actions, server orders, assisted owner staging and cooperative AI lead are explicit. Ordinary controls, audible radio and multiplayer remain unproved.
