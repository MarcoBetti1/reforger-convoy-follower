# Sampled-head supply V4: complete supply sequence, spacing still fails

September 27, 2026. **`pair_complete=true`; overall physical/raw/strict FAIL.** Both original trucks loaded, transported and delivered 100 supplies, completed real Hold, and independently qualified fresh Resume. Spacing remains outside the unchanged 60 m gate; the candidate is private and unpromoted.

| Evidence | Closed result |
| --- | --- |
| Original-container ledger | **467** valid rows conserve 1,800; final **1,600/0/0/200** |
| Arrival / real Hold | **180.426 s /180 samples each**, then **51.0648 s /51 each**, zero drift |
| Fresh head Resume | First qualified **20.853023 m /5 powered**; same-sequence best **50.747653 m /7** |
| Fresh tail Resume | **25.206492 m /9 powered**, genuine joined guide/epoch 1, net 24.054 m |
| Lead restart | **54.9133 m /11 powered** |
| Peak spacing | **128.138 m head /75.0655 m tail** — both fail |

## The new direct-arrival qualification was not needed

Terminal **01:20:57.015** followed two independently verified **moving** sequences: head 5 and tail 6. There was no new-leg admission or pooled credit. The passive Resume handoff receipt appeared **966 ms after terminal**; it earned nothing. V4's added direct-arrival gate remains **unexercised**, and [V3's failure](sampled-head-supply-integration-v3.md) is not reclassified.

The pending-guide observation branch executed, and the full log contains no trail block or post-result fixture failure. On-foot-owner and unowned-AI Resume calls were rejected without losing Hold or assignments; passenger Resume was accepted after the held seat handoff. Original identities, actual predecessor chain and exact native cargo containers stayed intact. Baseline continuous movement was 197.9576 m /28 powered head and 240.40293 m /30 tail.

## Attribution and limits

All-five validation/packaging passed; **587 frozen sources /three deployments** verified without mismatch. Only two declared fixture files differ from V3, including passive runtime instrumentation of the production handoff method. Other 585 resource hashes match. Pack **`04EB75881CA3DD32DEB7471F713D3EF30C100EFB6E6FC29DD775E7AE3EF5B93D`**; manifest **`39D979CCA60092479AB4D7239DB362261920B7386A62AAF667DF744CEE6B1E4E`**.

Errors remain **9 gameplay /0 post-result /64 shutdown**. Exact-prefix snapshot **+98 ms** and natural shutdown after 30.0155 s are retained. Root consumed all three handles with exit 0 and verified no matching game/Workbench/recorder processes or windows. The silent private video is **498.333333 s /7,474 frames**, independently verified SHA **`DBD71952E5EB90158DD4D07BCEE8C755B3D0BB4CA8EE70F3793D25B948AA4869`**. Three live views were sampled, including post-result parking during grace; no full playback.

Evidence: [independent review](../../.cache/client/runs/sampled-head-supply-integration-v4/independent-review.md), [analysis](../../.cache/client/runs/sampled-head-supply-integration-v4/independent-review.json), [source/deployment checks](../../.cache/client/runs/sampled-head-supply-integration-v4/independent-source-verification.json) and [closure/video](../../.cache/client/runs/sampled-head-supply-integration-v4/root-closure-video.json). Full console SHA `CDE4160BB9179DD835E941F1E8A49C0FFD28F0D99FD53D3DE1F62E769C5DB558`; snapshot SHA `C58ABB78B98940665E3C7E9F72DADE18A3E5EFC8214EAB8787BDC7C5579F1315`.

**Source replay:** first reconstruct the [exact V3 archive](sampled-head-supply-integration-v3.json), then apply the [two-file V4 patch](sampled-head-supply-integration-v4.patch) with `git -c core.autocrlf=false apply --binary` from that copy's root containing `addons`. The [small manifest](sampled-head-supply-integration-v4.json) pins both baseline/target hashes. Isolated replay verified all 587 resources: exactly two changed, other 585 byte-identical. Patch SHA **`509E3AB69066E1451BF64C310BEC146326290C7859861896EEC0F9437F1348BA`**; patch/JSON retain `-text` protection. These fixtures remain experimental.

This is assisted native-action/server-order evidence with staged owner and native-AI lead. It does not prove ordinary input, Reboard invocation, multiplayer, repeatability or release readiness. Preserve all failed spacing gates and earlier results; no further V4 observer iteration is required by this report.
