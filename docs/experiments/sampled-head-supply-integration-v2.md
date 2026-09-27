# Sampled-head supply V2: delivery and Hold, restart blocked

September 27, 2026. **Raw/physical/strict FAIL; private, unpromoted.** V2 changes only the tail's conservative prejoin-prefix retention check over [V1](sampled-head-supply-integration-v1.md). The sampled head policy, original assignments, native cargo actions, course and all physical gates remain unchanged.

| Stage | Independent closed-run evidence |
| --- | --- |
| Native supply | **100 loaded and unloaded per original truck; 200 delivered** |
| Conservation | **459** valid original-container rows, total 1,800; final **1,600/0/0/200** |
| Exact continuous baseline travel | Head **220.6612 m /26 powered intervals**, tail **238.1761 m /32** |
| Arrival / real Hold | **180.446 s /180 samples** and **51.1162 s /51 each**, zero drift |
| Peak spacing | Head **106.677 m**, tail **72.0678 m** — both exceed 60 m |
| Fresh Resume | **Neither follower qualified**; tail blocked during restart |

The tail genuinely joined epoch 1 at **00:38:13.164**, `forced=false`. There are **no `TRAIL_PREJOIN_RETAIN` events**, so this does not prove the new branch executed. Compared with the [earlier complete loop](road81-integrated-trip-v1.md), head peak decreased from 172.483 m while tail peak worsened from 44.9963 m. Preserve both failures; the tradeoff does not justify promotion.

Both unloads completed under real Hold. On-foot owner and unowned-AI Resume calls were rejected with Holds/identities unchanged; passenger Resume was then accepted. At **00:43:57.780**, Unit2 failed `reverse_physical_context_unavailable` with epoch/history retained. Terminal **00:43:57.781** correctly reports `pass=false pair_complete=false`. The subsequent generic identity/session summary is not independent evidence that an original assignment changed.

Head's best fresh sequence 5 progress was **3.07408 m /one powered interval at that best**; final progress 2.73134 m had six powered intervals. Tail earned **zero fresh eligible movement**, despite 3.87938 m net displacement. Lead reached 55.537 m /11 powered intervals. No baseline or separate-sequence credit was pooled into Resume.

## Attribution and closure

All-five validation/packaging passed. **587 frozen sources /three deployed files** verified without mismatch. Only Trail controller **`2FFB87114406D00E412A82DBF5D8A376CA43E451C94584E5B2D7A37D9DEB63F7`** differs from V1; the other 586 source hashes match. Pack **`9B0D072ED5C9D5AADA1CA6346221491BF4F4407517E1C9C3DEA50370C419147F`**; source manifest **`89CCEC4CF29DA66310C24A3D3522650A1E9C14EFF9A7D7E38843A14906C08E81`**.

Full errors remain **9 gameplay /0 post-result /64 shutdown**. Terminal snapshot is the exact prefix at **+220 ms**; native closure followed the 30-second grace period. Root consumed all three handles with exit 0 and recorded no matching processes/game windows. Recording **789.666667 s /11,844 frames**, SHA **`B836C98E0A7BFF250D4CA03192288DFDA249497BC1A0D275DA71F2B45CF6508E`**, independently matches. Four live views were sampled; the last was just before terminal. No terminal image or full playback. The silent raw video includes desktop footage and remains private.

Evidence: [independent review](../../.cache/client/runs/sampled-head-supply-integration-v2/independent-review.md), [analysis](../../.cache/client/runs/sampled-head-supply-integration-v2/independent-review.json), [source/deployment checks](../../.cache/client/runs/sampled-head-supply-integration-v2/independent-source-verification.json), [closure/video](../../.cache/client/runs/sampled-head-supply-integration-v2/root-closure-video.json). Full console SHA `D78742F44A5AD01826A3769CD75FBC7FB2E415AF87C091D976A218E9C7D41577`; snapshot SHA `7DBF85FB0C3C12050308F8ADBF7CC89210FC0BEDA7A2ED49366E579DF5095F0A`.

**Next:** a narrow V3 fix for physical observation while the guide is pending is under source peer review, not yet built. Preserve every gate and the earlier completed journey/demo. This run proves assisted native delivery and Hold, not complete Resume, ordinary input, a Reboard invocation or multiplayer.
