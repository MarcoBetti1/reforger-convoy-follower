# Sampled head in the supply loop: tail entry blocked

September 27, 2026. **Raw/physical/strict FAIL; private, unpromoted.** This complete-trip attempt combined the sampled head target (15 m refresh, native5) with the current canonical tail/Reboard baseline and six established Road81 fixtures. It preserved the actual predecessor chain and all cargo, spacing, arrival, Hold and fresh Resume gates.

| Stage | Closed-run evidence |
| --- | --- |
| Native loading | **100 into each original truck**; both cancelled stably |
| Four-container ledger | **109 valid rows**, total 1,800; final **1,600/100/100/0** |
| Partial powered travel | Head **42.7951 m /7** in exact sequence1; lead **66.9363 m /13**; tail **no joined-route credit** |
| First failure | **00:29:30.196**, Unit2 epoch1 `prejoin_window_history_changed` |
| Arrival / Hold / delivery / Resume | **Not reached** |

At 00:29:30.065 the tail was **2.96218 m** from the original recorded entry, with facing **0.931935**, but remained unjoined. The subsequent block retained history and refused a forced join. Later fixture summaries do not establish a separate assignment loss. Terminal **00:29:31.146** correctly reported `pass=false pair_complete=false`; the **44.8457 m** early peak is not a completed-course spacing pass. This run does not establish whether the faster head policy improves a complete two-truck journey.

## Attribution and closure

All-five validation and packaging passed. Independent checks verified **587 frozen sources /three deployed files**, all 11 declared overlays and protected dependencies, with no attribution issues. Pack **`F683F18D377E0485EAE4AD891C0086115FA62E49FA681A01DA564FC7704B0300`**; source manifest **`E6ADF508A1821B2537DE150F2D0A1EB75F13226AD4B812D073CB2A673E713F74`**. Exact candidate and frozen source remain under `.cache/sampled-head-supply-integration-v1` and the run directory.

Full errors remain **9 gameplay /0 post-result /64 shutdown**. The exact-prefix terminal snapshot was captured **+131 ms**; native shutdown followed the 30-second grace period. Root consumed client/watcher/recorder with exit0 and found no remaining processes/windows. The **153.933333 s /2,308-frame** silent recording independently matches SHA **`9BD21BAF7E89DCF373F21C664776C1B2986FDF349ADFDADAAA655E3B88202409`**. Root inspected loading and a post-terminal grace-period view; no full playback or exact-terminal-frame claim.

Evidence: [independent review](../../.cache/client/runs/sampled-head-supply-integration-v1/independent-review.md), [analysis](../../.cache/client/runs/sampled-head-supply-integration-v1/independent-review.json), [source/deployment checks](../../.cache/client/runs/sampled-head-supply-integration-v1/independent-source-verification.json) and [closure/video](../../.cache/client/runs/sampled-head-supply-integration-v1/root-closure-video.json). Full console SHA `203B7EE50A3B3046E451A811484F4B58F1043F5A7006B583A64E1F74ABEFE6C6`; snapshot SHA `053C45B0ADAE2FD74BFF32C00D29F599C8471CB2ED3F58AAB653436E0E4BB459`.

**Next:** source review found that a larger prejoin-window request can return an earlier whole endpoint after rejecting a farther point on the same segment, while the already-offered partial point remains valid. The exact second-request geometry was not logged. A private repair candidate retains the unchanged offered point only if the same reader and exact `ReadRecorded` origin/direction validate it; compilation and live proof are pending. Genuine physical join and all gates remain required. Return to the complete supply loop, preserving the earlier [completed automated journey and demo](road81-integrated-trip-v1.md). Ordinary controls and release readiness remain open.
