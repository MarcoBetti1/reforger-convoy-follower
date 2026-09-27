# Canonical three-follower mixed-road regression

**27 September 2026 — closed FAIL.** The unchanged ordinary three-follower course retained the original driver/truck chain and established powered travel for all three followers. Two spacing links exceeded60m, and the second follower's native stopping approach left the retained route corridor before formal arrival observation began.

| Evidence through terminal | Result |
|---|---|
| Head exact moving activity | 324.831m /28 powered intervals, sequence1 |
| Unit2 exact moving activity | 289.150m /34 powered intervals, sequence2; genuine guide join, epoch1 |
| Unit3 exact moving activity | 312.317m /35 powered intervals, sequence3; genuine guide join, epoch1 |
| Sampled peak adjacent gaps | Head94.599m; Unit2 54.5513m; Unit3 63.6727m |
| Arrival | Zero formal180s observation samples; head captured before terminal, Unit3 only afterward |
| Moving natural-material samples | Lead10, head11, Unit2 14, Unit3 13; at least4 distinct wheels and speed≥1km/h per sample |
| Full lifecycle errors | 9 gameplay /0 post-result /59 shutdown |

The wheel records establish some actual dirt-road travel for all four vehicles, with grass/soil/dirt contacts also recorded for individual followers. They do not establish an entirely unpaved route or off-network driving. The native lead used the existing20km/h request, without the cooperative cohesion-advice consumer. Cargo and explicit Hold/Resume were outside this course.

## Concrete stopping failure

The first sticky failure was head spacing at03:58:22.425. Unit2 changed from its guide to the real predecessor's10m stopping approach at03:58:58.709; sequence5 remained the healthy selected native activity. The head captured arrival at03:59:24.776. During Unit2's subsequent forward/reverse parking, its sampled route cross-track reached4.79314m at03:59:47.209. At03:59:47.342 the retained-pose query produced **OFF_ROUTE (state4)** and the controller blocked that still-active arrival order. This is not ADVANCE_LIMIT (state8); no preceding advance-failure diagnostic appears in the log.

Terminal03:59:48.209, world150.348s, retained `PACED_RESULT: FAIL`, peak94.599m and zero observation samples. Unit3's later capture at03:59:58.109 cannot supply the missing arrival proof. The bounded arrival-recovery handoff was not exercised.

The next source decision is the boundary between native real-predecessor parking and retained-route observation. Preserve the recorded history and departure guards while reviewing whether an ordinary local stopping maneuver should become a permanent route failure. This run does not support increasing the corridor or treating unverified reacquisition as a successful join.

## Reproduction and limits

Canonical commit `143e091`; unchanged world `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_OrdinaryMixed_3Trucks.ent`. All590 frozen source files and all3 deployed files independently matched. Pack: `3CC40540C30BE9CAEF37499424809B57DD7A3C8B3C7340EF4AC6B91CAB706F1D`; source manifest: `32C66F7B49BDC169F2F99EFCA4D369DBB0384AEA87AAAE7A0E31C1420F40702F`. No experimental source archive is required.

Private evidence is under `.cache/client/runs/arrival-canonical-three-v1/`: full console, terminal prefix, independent review and root closure. The terminal prefix matched exactly at+177ms. All four process handles closed with exit0 and fresh process/window checks were empty. Video230.733333s and audio242.6s hashes were independently verified. Two live views covered departure and approach; the attempted terminal screenshot missed the already-closed game. Full playback and audio content were not reviewed.

The legacy analyzer's `arc = end − progress` check is **unassessed** here: the frozen observer reports monotonic high-water progress but local-station arc, without the fields needed to reconcile reverse motion. This is not an additional demonstrated fault or a waived gate. Raw spacing and arrival failures remain decisive. Assisted native-AI travel does not prove ordinary player input, multiplayer or release readiness.
