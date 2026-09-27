# Road81 supply trip: entry and arrival worked; delivery incomplete

September 26, 2026. **Closed physical FAIL; strict FAIL.** The one-file native entry approach was tested on the same [Road81 loaded-pair course](road81-loaded-trip-v1.md). It preserved original trucks, drivers, predecessor history, native cargo actions and every physical gate. No driving candidate is promoted by this result.

## Useful progress and remaining failures

- The initial diagnostic measured **4.90958 m lateral offset** from the predecessor's first 0.541268 m recorded segment. The waiting truck faced correctly (dot 0.971761) but was 19.9921 m from entry. Native planning approached the **original recorded point**, without inventing a route or joining early.
- Physical capture followed 6.4664 s later: distance 2.96108 m, lateral offset 1.12998 m and facing dot 0.92586, still unjoined. The unchanged route query then genuinely joined epoch 1 at 21:48:24.365, `forced=false`. No subsequent route block occurred.
- Both original followers drove and captured arrival. Continuous travel under one exact activity was **157.651 m /26 powered intervals** for the head and **229.631 m /39** for the tail. All 699 driving/observation identity rows retained the original chain.
- **Spacing failed:** peak links **158.345 /88.2234 m**, against 60 m. First failure was 21:48:25.333. Successful entry and eventual arrival do not excuse those gaps.
- Both trucks completed **180.346 s arrival observation**, then real Hold for at least **47.0655 s**, with zero measured drift. On-foot-owner and unowned-AI Resume attempts were rejected safely. The terminal `hold_s=0` field was never finalized because Resume was not reached; 47 measured Hold samples per truck establish the actual window.
- Native loading put 100 in each truck. The head delivered 100; the tail remained **14.0528 m from storage**, and its native NoStorage discovery timed out after 5 s. Final original counts were **source 1600 /head 0 /tail 100 /destination 100**, conserved across 447 ledger rows. **No 200-supply delivery or Resume was completed.**

The 60 m onward road walk qualified natively at 1611.2/43.0379/3087.36, but was never driven. The next complete-trip attempt needs an authored destination reachable from both measured held positions and a pacing decision that addresses the large gaps. Keep the native range check and the 60 m spacing, 180 s arrival, 2 m drift, 30 s Hold and fresh 20 m /three-powered-interval gates unchanged.

## Reproduction and evidence

The [exact source patch](road81-entry-approach-v1.patch) applies after the frozen Road81 fixture/V5 source chain, not an arbitrary canonical checkout. It changes only `Scripts/Game/Tests/CF_TrailGuideDriverControllerComponent.c`, SHA `A698C2D256DDAEF3195B7F02EA22DDC52BC7C32C661DC342749B2812D7E7B02E`; isolated apply-check and byte-exact reconstruction passed. No Reboard files are included.

- World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`. Five configurations and packaging passed in `.cache/workbench-runs/convoy-road81-entry-approach-v1`.
- Pack `EE3B4C3DEDFE7D8650C911EBE75982A8B460E713042DBE53A9B76E12B80460EA`; manifest `8716CCF06252047825531D6F06AE7161A8EB01925970AA6B7BC4A2793068B3D5`. **588 sources/three deployment files verified.**
- Terminal 21:53:52.083; exact-prefix snapshot +86 ms. **9 gameplay /0 post-result /64 shutdown errors retained.** Natural closure, all three consumed handles (exit 0) and an empty process check were verified.
- Recording 500.466667 s /7506 frames, SHA `9BA08FFA42DE10FD7DECB8032339E7FC09791B7632725F4BCEB7A61453E9BFFE`. Staging and stopped-arrival views were inspected; no terminal screenshot or full playback.
- [Independent review](../../.cache/client/runs/road81-entry-approach-v1/independent-review.md), SHA `2784C198F421DC8DFA0F3D0AFD9D65200FA0D7E11C277FB096A8BB59D5EB7A4B`; structured report, full log and closure records are adjacent.

Test-owner staging and native AI driving remain explicit assistance. Ordinary keyboard/menu input, multiplayer and release readiness are not proved.
