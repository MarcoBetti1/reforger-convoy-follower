# Cooperative leader trip V2 — recovery branch unexercised

September 27, 2026 — **closed FAIL; private, not promoted.** V2 changes only the bounded arrival-recovery handoff from [V1](cooperative-cohesion-trip-v1.md). An earlier native tail request failed before that branch could run.

| Stage | Observed result |
|---|---|
| Native cargo | 100 loaded into each original truck. All **176** full-log ledger rows conserve 1,800; final source/cargo1/cargo2/destination **1,600 /100 /100 /0**. **No unloading or delivery.** |
| Powered travel | Lead **233.860672 m /28** powered intervals; head original sequence1 **227.784407 m /40**; genuinely joined tail guide sequence2 **197.991700 m /27**. Original assignments retained. |
| Peak gaps | **46.5687 m head /40.9363 m tail**, below 60 m during the observed travel. Aborting early does not establish a full-course spacing pass. |
| Arrival and commands | No formal arrival observation, 180 s arrival, explicit Hold or Resume. `pair_complete=false`. |
| Full errors | **9 gameplay /0 post-result /62 shutdown**, retained. |

## Failure and useful evidence

At **02:08:19.474**, tail sequence2's original lease failed with `guard_or_request_failure_admitted_move_3_handler_1`; `ENTITY_FOLLOW_FALLBACK_FAILED` followed. The terminal at **02:08:20.091**, world136.114 s, reported `private_controller_missing_or_failed_unit_2`. The prior sample still had the exact joined guide, route state2 and cross-track 1.51893 m. No trail-projection block was logged; the exact native path-planner cause remains unproved.

There were **no recovery admission, final-advance or fresh-approach events**. The new same-predecessor arrival handoff is therefore **unexercised**, not a demonstrated repair or failure. The head captured its arrival Wait at02:08:40.274, after the result; that supplies no pre-terminal credit.

Cooperative pacing again kept observed travel gaps much smaller than the preceding constant-ceiling canonical course, but the loop remains incomplete. The first advice warning occurred at **44.3469 m**, reducing the cached cap to11.5487 km/h. Across112 samples (91 valid/21 unavailable), unavailable advice never increased the cap; freshness and upward-rate checks passed.89 next-poll owned native-cruise readbacks matched within the existing1 km/h update tolerance. Radio audibility, level2 WAIT advice, explicit Hold/Resume consumption and the retired-action deferral were not demonstrated. No constant-speed following claim follows from a cooperative leader.

## Reproduction and evidence

World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`. All-five validation/packaging passed; **585 frozen sources /3 deployment files** verified. The sole V1→V2 source change is the ordinary controller, SHA `19B49FBF927CA32826F620E11574C451D015B9AD961FD303D8EE6AA980BF048A`.

- Pack: `E71C84A81F8C508B282DB7998895F68A858F335B118557E16C6554BD0C1F6EB7`.
- Manifest: `4D3B747F6F6781ECDF6CB9DEB95FD06F8AE465B1684C673661F63658CBCE4E0F`.
- [Independent review](../../.cache/client/runs/cooperative-cohesion-trip-v2/independent-review.md), [analysis JSON](../../.cache/client/runs/cooperative-cohesion-trip-v2/independent-review.json), [full log](../../.cache/client/runs/cooperative-cohesion-trip-v2/logs/console.log), [closure record](../../.cache/client/runs/cooperative-cohesion-trip-v2/root-closure-video.json).
- [Source patch](cooperative-cohesion-trip-v2-source.patch) and [replay manifest](cooperative-cohesion-trip-v2-source.json) preserve the full inherited candidate against raw Git `556dfce`; their scope is larger than V2's single increment.

The terminal snapshot is an exact log prefix captured **+23 ms**. Client, watcher and recorder closed naturally with exit0; fresh process/window checks were empty. The [silent raw video](../../.cache/test-videos/cooperative-cohesion-trip-v2-full.mp4) is216.733333 s /3,250 frames, independently verified SHA `A69A0AEABEAD94D6F5F8623E4B39104D2EE3194BD3017834AA844F2F3DEB24B3`. Root sampled two live views; no terminal screenshot or full playback. Scripted native actions, assisted owner staging and native AI lead remain explicit. Ordinary controls and multiplayer remain unproved.
