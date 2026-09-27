# Natural failure recovery and supply delivery

**27 September 2026 — closed FAIL, with a recovered delivery milestone.** After a real native tail order failed, public Hold/Resume commands returned the same convoy to its original destination and delivered all 200 supplies. The later departure failed a different tail route guard. Neither the original failure nor the incomplete final Resume is waived.

| Gate | Observed result |
|---|---|
| Native loading and delivery | 100 loaded and unloaded by each original truck; destination 0→200 |
| Conservation | 512 terminal /541 full-log records; same four containers, total 1800, no discrepancies; final source1600/trucks0+0/destination200 |
| Natural command recovery | Actual native move3 failure retained; real Hold30.0994s/30 samples, zero drift; Resume continued to the unchanged original delivery goal |
| Arrival | 180.396s, 180 exact captured-Wait samples per driver, zero drift |
| Delivery Hold | 51.0818s/51 samples per driver, zero drift; on-foot owner and unowned-AI Resume calls rejected without changing Holds |
| Final fresh Resume | Head sequence7 qualified24.67070m/7 powered intervals; tail sequence8 reached only7.78427m/5, below20m |
| Spacing | Maximum through terminal49.2778m, below the unchanged60m limit |
| Raw lifecycle errors | 9 gameplay /0 post-result /64 shutdown |

The first failure occurred at **03:19:41.490**, `guard_or_request_failure_admitted_move_3_handler_1`, with a16.2188ms-old exact executing-lease witness. This run injected no failure. Hold was accepted at03:19:43.272 and Resume at03:20:16.406. Both trucks subsequently captured arrival and delivered using the native actions. Original drivers, trucks, predecessor chain and tail route epoch1 were retained; recovery did not reset the failed lease or extend the delivery goal.

At **03:25:30.604**, the final departing tail blocked with `local_turn_over_90_degrees`; terminal followed at**03:25:30.606**. Its fresh sequence had7.784272m of independently summed eligible motion,5 powered intervals and7.40828m net progress. Head proof was independently reproduced, but no tail qualification or complete-workflow success was earned. Raw `workflow_complete`, `workflow_recovered` and `overall_pass` all remain false. Physical recovery to delivery is a narrower result than those terminal gates.

## Reproduction and inspection

The [six-resource incremental patch](natural-recovery-trip-v1-source.patch) and [manifest](natural-recovery-trip-v1-source.json) apply after the exact [preceding full-supply source archive](cooperative-supply-recovery-v1-source.json). Two observer files changed and four new probe/world resources were added; all product bytes and588 other resources were unchanged. Isolated application reproduced all six changed byte hashes with no extra paths. Frozen source594/594 and deployment3/3 matched. World: `Worlds/Tests/ConvoyFollower_Arland_NaturalRecoverySupplyTrip_2Trucks.ent`. Pack: `EA4BE53E9841133E2EEA5A200CAABB717A003B22A5881861A50CC117F0068A8F`.

The terminal snapshot was an exact full-log prefix at+147ms. Client, watcher, video and audio closed with exit0; native shutdown and empty process/window checks were retained. Full video571.266667s and audio583.3s were hash-verified. Seven live views included terminal+2.081s. A123s edited demonstration was produced and its nine-frame contact sheet inspected; full playback and audio content/synchronization remain unreviewed. Raw output-mix media is not published here.

This is an assisted native-AI lead, native cargo-action and server-command test. It does not establish ordinary player input, multiplayer, repaired native navigation, a clean lifecycle or release readiness. The useful next repair concerns the final tail route rejection; delivery and command recovery should remain regression gates.
